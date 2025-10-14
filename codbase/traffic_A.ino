/*
 * SEMAFORO_A - Sistema de Control Semafórico Inteligente
 * ESP32-S3 con ESP-NOW, HC-SR04, OLED y LEDs
 * Universidad Militar Nueva Granada - Bluetooth y NOW
 * 
 * Este código controla el SEMAFORO A en una intersección de dos vías.
 * Comunicación con SEMAFORO B mediante ESP-NOW para sincronización.
 * 
 * HARDWARE:
 * - ESP32-S3
 * - 3 LEDs (Rojo, Amarillo, Verde) con resistencias 220Ω
 * - Sensor ultrasónico HC-SR04
 * - Pantalla OLED I2C 128x64 (SSD1306)
 * - Buzzer opcional
 * 
 * LIBRERÍAS REQUERIDAS:
 * - Adafruit_SSD1306
 * - Adafruit_GFX
 * - esp_now.h (incluida en ESP32 core)
 */

#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== CONFIGURACIÓN DE PINES ====================
#define LED_ROJO      16
#define LED_AMARILLO  17
#define LED_VERDE     18

#define TRIG_PIN      25
#define ECHO_PIN      26

#define BUZZER_PIN    27  // Opcional

#define OLED_SDA      21
#define OLED_SCL      22
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// ==================== IDENTIFICACIÓN ====================
#define DEVICE_ID     1   // 1 = SEMAFORO_A, 2 = SEMAFORO_B

// Dirección MAC del SEMAFORO_B (reemplazar con la MAC real del otro ESP32)
// Obtener con WiFi.macAddress() en el otro dispositivo
uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // CAMBIAR ESTO

// ==================== PARÁMETROS DEL SISTEMA ====================
#define GREEN_NORMAL        10000  // 10 segundos en verde (modo normal)
#define YELLOW_DURATION     3000   // 3 segundos en amarillo
#define ALL_RED_DURATION    1000   // 1 segundo con ambos en rojo (seguridad)
#define MAX_GREEN           20000  // Máximo tiempo en verde (modo adaptativo)
#define EXTEND_STEP         3000   // Incremento por detección (3 s)

#define DETECTION_THRESHOLD_CM  100   // Distancia para detectar vehículo (cm)
#define DETECTION_PERSIST_MS    1000  // Tiempo de confirmación de detección
#define BROADCAST_INTERVAL      200   // Intervalo de envío ESP-NOW (ms)
#define PEER_TIMEOUT            2000  // Tiempo sin mensajes para modo seguro

// ==================== ESTADOS DEL SEMÁFORO ====================
enum TrafficState {
  STATE_ALL_RED = 0,
  STATE_GREEN = 1,
  STATE_YELLOW = 2,
  STATE_RED = 3,
  STATE_WAIT = 4
};

// ==================== ESTRUCTURA DE MENSAJE ESP-NOW ====================
struct __attribute__((packed)) TrafficMsg {
  uint8_t sender_id;      // ID del emisor (1 o 2)
  uint8_t seq;            // Contador de secuencia
  uint8_t state;          // Estado actual (0-4)
  uint8_t request;        // Solicitud de prioridad (0=no, 1=sí)
  uint16_t distance_cm;   // Distancia medida en cm
  uint32_t timestamp_ms;  // Tiempo local en ms
};

// ==================== VARIABLES GLOBALES ====================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

TrafficState currentState = STATE_ALL_RED;
TrafficState remoteState = STATE_ALL_RED;

unsigned long stateStartTime = 0;
unsigned long lastBroadcast = 0;
unsigned long lastRemoteMsg = 0;
unsigned long lastDetectionTime = 0;

uint16_t currentDistance = 9999;
bool vehicleDetected = false;
bool requestPriority = false;
bool remoteRequestPriority = false;
uint16_t remoteDistance = 9999;

uint8_t messageSeq = 0;
uint8_t remoteSeq = 0;

int greenDuration = GREEN_NORMAL;
int cycleCount = 0;
int priorityCount = 0;

// ==================== FUNCIONES DE LED ====================
void setLEDs(bool red, bool yellow, bool green) {
  digitalWrite(LED_ROJO, red ? HIGH : LOW);
  digitalWrite(LED_AMARILLO, yellow ? HIGH : LOW);
  digitalWrite(LED_VERDE, green ? HIGH : LOW);
}

void updateLEDsForState() {
  switch(currentState) {
    case STATE_ALL_RED:
      setLEDs(true, false, false);
      break;
    case STATE_GREEN:
      setLEDs(false, false, true);
      break;
    case STATE_YELLOW:
      setLEDs(false, true, false);
      break;
    case STATE_RED:
      setLEDs(true, false, false);
      break;
    case STATE_WAIT:
      setLEDs(true, false, false);
      break;
  }
}

// ==================== FUNCIONES DE SENSOR HC-SR04 ====================
uint16_t measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms
  if (duration == 0) return 9999; // Sin eco
  
  uint16_t distance = duration * 0.034 / 2;
  return distance;
}

void updateVehicleDetection() {
  currentDistance = measureDistance();
  
  if (currentDistance < DETECTION_THRESHOLD_CM) {
    if (!vehicleDetected) {
      lastDetectionTime = millis();
    }
    // Confirmar detección sostenida
    if (millis() - lastDetectionTime >= DETECTION_PERSIST_MS) {
      vehicleDetected = true;
      requestPriority = true;
    }
  } else {
    vehicleDetected = false;
    if (currentState != STATE_GREEN) {
      requestPriority = false;
    }
  }
}

// ==================== FUNCIONES DE PANTALLA OLED ====================
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Título
  display.setCursor(0, 0);
  display.print("SEMAFORO A");
  
  // Estado actual
  display.setCursor(0, 12);
  display.print("Estado: ");
  switch(currentState) {
    case STATE_ALL_RED: display.print("ALL RED"); break;
    case STATE_GREEN:   display.print("VERDE"); break;
    case STATE_YELLOW:  display.print("AMARILLO"); break;
    case STATE_RED:     display.print("ROJO"); break;
    case STATE_WAIT:    display.print("ESPERA"); break;
  }
  
  // Tiempo restante
  unsigned long elapsed = millis() - stateStartTime;
  unsigned long remaining = 0;
  switch(currentState) {
    case STATE_GREEN:
      remaining = (greenDuration > elapsed) ? (greenDuration - elapsed) / 1000 : 0;
      break;
    case STATE_YELLOW:
      remaining = (YELLOW_DURATION > elapsed) ? (YELLOW_DURATION - elapsed) / 1000 : 0;
      break;
    case STATE_ALL_RED:
      remaining = (ALL_RED_DURATION > elapsed) ? (ALL_RED_DURATION - elapsed) / 1000 : 0;
      break;
  }
  display.setCursor(0, 24);
  display.print("Tiempo: ");
  display.print(remaining);
  display.print(" s");
  
  // Detección de vehículo
  display.setCursor(0, 36);
  if (vehicleDetected) {
    display.print("VEH: SI (");
    display.print(currentDistance);
    display.print("cm)");
  } else {
    display.print("VEH: NO");
  }
  
  // Estado remoto
  display.setCursor(0, 48);
  display.print("Via B: ");
  switch(remoteState) {
    case STATE_GREEN:   display.print("VERDE"); break;
    case STATE_YELLOW:  display.print("AMAR"); break;
    case STATE_RED:     display.print("ROJO"); break;
    default:            display.print("???"); break;
  }
  
  // Sincronización
  display.setCursor(0, 56);
  if (millis() - lastRemoteMsg > PEER_TIMEOUT) {
    display.print("SIN SYNC");
  } else if (requestPriority) {
    display.print("PRIO A");
  } else if (remoteRequestPriority) {
    display.print("PRIO B");
  } else {
    display.print("OK");
  }
  
  display.display();
}

// ==================== CALLBACKS ESP-NOW ====================
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Callback opcional para debug
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Error enviando mensaje ESP-NOW");
  }
}

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(TrafficMsg)) {
    Serial.println("Mensaje de tamaño incorrecto");
    return;
  }
  
  TrafficMsg msg;
  memcpy(&msg, incomingData, sizeof(msg));
  
  // Actualizar estado remoto
  remoteState = (TrafficState)msg.state;
  remoteRequestPriority = (msg.request == 1);
  remoteDistance = msg.distance_cm;
  remoteSeq = msg.seq;
  lastRemoteMsg = millis();
  
  Serial.print("RX de ESP ");
  Serial.print(msg.sender_id);
  Serial.print(": estado=");
  Serial.print(msg.state);
  Serial.print(", request=");
  Serial.print(msg.request);
  Serial.print(", dist=");
  Serial.println(msg.distance_cm);
}

// ==================== FUNCIONES ESP-NOW ====================
void initESPNow() {
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
  
  // Registrar peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error añadiendo peer");
  } else {
    Serial.println("Peer añadido correctamente");
  }
}

void broadcastState() {
  if (millis() - lastBroadcast < BROADCAST_INTERVAL) {
    return;
  }
  
  TrafficMsg msg;
  msg.sender_id = DEVICE_ID;
  msg.seq = messageSeq++;
  msg.state = (uint8_t)currentState;
  msg.request = requestPriority ? 1 : 0;
  msg.distance_cm = currentDistance;
  msg.timestamp_ms = millis();
  
  esp_err_t result = esp_now_send(peerMAC, (uint8_t *)&msg, sizeof(msg));
  lastBroadcast = millis();
  
  if (result == ESP_OK) {
    Serial.print("TX: estado=");
    Serial.print(msg.state);
    Serial.print(", request=");
    Serial.print(msg.request);
    Serial.print(", dist=");
    Serial.println(msg.distance_cm);
  }
}

// ==================== LÓGICA DE CONTROL ====================
bool canTransitionToGreen() {
  // Verificar que el otro semáforo no esté en verde
  if (remoteState == STATE_GREEN || remoteState == STATE_YELLOW) {
    return false;
  }
  
  // Si hay timeout de comunicación, ser conservador
  if (millis() - lastRemoteMsg > PEER_TIMEOUT) {
    Serial.println("Sin comunicación con peer - modo seguro");
    return false;
  }
  
  return true;
}

bool shouldGetPriority() {
  // Si ambos solicitan prioridad, decidir por distancia
  if (requestPriority && remoteRequestPriority) {
    if (currentDistance < remoteDistance - 10) {
      return true;  // Este carril tiene vehículo más cercano
    } else if (abs((int)currentDistance - (int)remoteDistance) < 10) {
      // Distancias similares, usar ID como desempate
      return DEVICE_ID < 2;
    } else {
      return false;
    }
  }
  
  // Si solo este lado solicita prioridad
  if (requestPriority && !remoteRequestPriority) {
    return true;
  }
  
  return false;
}

void updateStateMachine() {
  unsigned long elapsed = millis() - stateStartTime;
  
  switch(currentState) {
    case STATE_ALL_RED:
      if (elapsed >= ALL_RED_DURATION) {
        // Decidir quién pasa a verde
        if (shouldGetPriority() || (cycleCount % 2 == 0)) {
          if (canTransitionToGreen()) {
            currentState = STATE_GREEN;
            greenDuration = vehicleDetected ? MAX_GREEN : GREEN_NORMAL;
            stateStartTime = millis();
            Serial.println("-> VERDE");
          }
        } else {
          currentState = STATE_RED;
          stateStartTime = millis();
          Serial.println("-> ROJO (turno de B)");
        }
      }
      break;
      
    case STATE_GREEN:
      // Extender verde si hay vehículo y no excede el máximo
      if (vehicleDetected && elapsed < MAX_GREEN && greenDuration < MAX_GREEN) {
        greenDuration = min(greenDuration + EXTEND_STEP, (int)MAX_GREEN);
      }
      
      if (elapsed >= greenDuration) {
        currentState = STATE_YELLOW;
        stateStartTime = millis();
        Serial.println("-> AMARILLO");
        tone(BUZZER_PIN, 1000, 200);  // Beep opcional
      }
      break;
      
    case STATE_YELLOW:
      if (elapsed >= YELLOW_DURATION) {
        currentState = STATE_RED;
        stateStartTime = millis();
        requestPriority = false;  // Reset solicitud
        cycleCount++;
        Serial.println("-> ROJO");
      }
      break;
      
    case STATE_RED:
      // Esperar a que el otro complete su ciclo
      if (remoteState == STATE_RED || remoteState == STATE_ALL_RED) {
        currentState = STATE_ALL_RED;
        stateStartTime = millis();
        Serial.println("-> ALL_RED (preparar cambio)");
      }
      break;
      
    case STATE_WAIT:
      // Estado de espera especial (no usado en esta implementación simple)
      break;
  }
  
  updateLEDsForState();
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SEMAFORO A - Iniciando ===");
  
  // Configurar pines
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  setLEDs(false, false, false);
  
  // Inicializar OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error inicializando OLED");
  } else {
    Serial.println("OLED OK");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("SEMAFORO A");
    display.println("Iniciando...");
    display.display();
  }
  
  // Inicializar ESP-NOW
  initESPNow();
  
  // Estado inicial
  currentState = STATE_ALL_RED;
  stateStartTime = millis();
  
  Serial.println("=== Sistema listo ===");
  tone(BUZZER_PIN, 2000, 100);
  delay(100);
  tone(BUZZER_PIN, 2500, 100);
}

// ==================== LOOP ====================
void loop() {
  // Leer sensor
  updateVehicleDetection();
  
  // Actualizar máquina de estados
  updateStateMachine();
  
  // Transmitir estado por ESP-NOW
  broadcastState();
  
  // Actualizar pantalla
  updateDisplay();
  
  // Pequeño delay para estabilidad
  delay(50);
}
