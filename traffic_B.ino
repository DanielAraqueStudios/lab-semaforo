/*
 * SEMAFORO_B - Sistema de Control Semafórico Inteligente
 * ESP32-S3 con ESP-NOW, HC-SR04, OLED y LEDs
 * Universidad Militar Nueva Granada - Bluetooth y NOW
 * 
 * Este código controla el SEMAFORO B en una intersección de dos vías.
 * Comunicación con SEMAFORO A mediante ESP-NOW para sincronización.
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
// Pines disponibles: 4,5,6,7,15,16,17,18,8,3,46,9,10,11,12,13,14
#define LED_ROJO      4
#define LED_AMARILLO  5
#define LED_VERDE     6

#define TRIG_PIN      7
#define ECHO_PIN      15

#define BUZZER_PIN    16  // Opcional

#define OLED_SDA      17
#define OLED_SCL      18
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// ==================== IDENTIFICACIÓN ====================
#define DEVICE_ID     2   // 1 = SEMAFORO_A, 2 = SEMAFORO_B

// MAC del peer se obtiene dinámicamente por UART
uint8_t peerMAC[6] = {0};

// Enviar propia MAC por UART
void sendOwnMAC() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Serial.print("MYMAC:");
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(":");
    Serial.printf("%02X", mac[i]);
  }
  Serial.println();
}

// Esperar y leer MAC del peer por UART
bool waitForPeerMAC(unsigned long timeoutMs = 5000) {
  unsigned long start = millis();
  String line;
  while (millis() - start < timeoutMs) {
    if (Serial.available()) {
      line = Serial.readStringUntil('\n');
      line.trim();
      if (line.startsWith("MYMAC:")) {
        int idx = 0;
        char *token = strtok((char*)line.c_str() + 6, ":");
        while (token && idx < 6) {
          peerMAC[idx++] = (uint8_t)strtol(token, NULL, 16);
          token = strtok(NULL, ":");
        }
        if (idx == 6) return true;
      }
    }
  }
  return false;
}

// ==================== PARÁMETROS DEL SISTEMA ====================
#define GREEN_NORMAL        10000  // 10 segundos en verde (modo normal)
#define YELLOW_DURATION     3000   // 3 segundos en amarillo
#define ALL_RED_DURATION    1000   // 1 segundo con ambos en rojo (seguridad)
#define MAX_GREEN           20000  // Máximo tiempo en verde (modo adaptativo)
#define EXTEND_STEP         3000   // Incremento por detección (3 s)

#define DETECTION_THRESHOLD_CM  5     // Distancia para detectar vehículo (cm)
#define DETECTION_PERSIST_MS    500   // Ventana de detección tras lectura válida
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
  
  // If any reading is within threshold, record the time of detection.
  if (currentDistance <= DETECTION_THRESHOLD_CM) {
    lastDetectionTime = millis();
  }

  // Consider vehicle detected for DETECTION_PERSIST_MS after the last detection reading.
  if (millis() - lastDetectionTime <= DETECTION_PERSIST_MS) {
    if (!vehicleDetected) {
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
  // OLED deshabilitado temporalmente
  return;
  
  /*
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Título
  display.setCursor(0, 0);
  display.print("SEMAFORO B");
  
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
  display.print("Via A: ");
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
    display.print("PRIO B");
  } else if (remoteRequestPriority) {
    display.print("PRIO A");
  } else {
    display.print("OK");
  }
  
  display.display();
  */
}

// ==================== CALLBACKS ESP-NOW ====================
// Nueva firma de callback de envío (core ESP32 reciente)
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  // Callback de confirmación de envío
  if (status == ESP_NOW_SEND_SUCCESS) {
    // TX exitoso - silencioso para no saturar logs
  } else {
    Serial.print("Callback: Error en envío, status=");
    Serial.println(status);
  }
}

// Nueva firma de callback de recepción (core ESP32 reciente)
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
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
  // Desconectar WiFi previo y limpiar configuración
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  // Configurar WiFi en modo estación
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);  // Máxima potencia
  delay(100);
  
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Desinicializar ESP-NOW si estaba activo
  esp_now_deinit();
  delay(100);
  
  // Inicializar ESP-NOW
  esp_err_t initResult = esp_now_init();
  if (initResult != ESP_OK) {
    Serial.print("Error inicializando ESP-NOW: 0x");
    Serial.println(initResult, HEX);
    return;
  }
  Serial.println("ESP-NOW inicializado OK");
  
  // Registrar callbacks
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
  
  // Verificar si el peer ya existe y eliminarlo
  if (esp_now_is_peer_exist(peerMAC)) {
    esp_now_del_peer(peerMAC);
    Serial.println("Peer anterior eliminado");
  }
  
  // Registrar peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  
  esp_err_t addResult = esp_now_add_peer(&peerInfo);
  if (addResult != ESP_OK) {
    Serial.print("Error añadiendo peer: 0x");
    Serial.println(addResult, HEX);
  } else {
    Serial.println("Peer añadido correctamente");
    Serial.print("Peer MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", peerMAC[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }
}

void broadcastState() {
  if (millis() - lastBroadcast < BROADCAST_INTERVAL) {
    return;
  }

  // Robust peer registration and send retry logic
  int sendAttempts = 0;
  const int maxSendAttempts = 2;
  esp_err_t result = ESP_FAIL;
  TrafficMsg msg;
  msg.sender_id = DEVICE_ID;
  msg.seq = messageSeq++;
  msg.state = (uint8_t)currentState;
  msg.request = requestPriority ? 1 : 0;
  msg.distance_cm = currentDistance;
  msg.timestamp_ms = millis();

  while (sendAttempts < maxSendAttempts) {
    // Ensure peer is registered
    if (!esp_now_is_peer_exist(peerMAC)) {
      Serial.println("[ESP-NOW] Peer not found, registering...");
      esp_now_peer_info_t peerInfo;
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, peerMAC, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      peerInfo.ifidx = WIFI_IF_STA;
      esp_err_t addResult = esp_now_add_peer(&peerInfo);
      if (addResult != ESP_OK && addResult != ESP_ERR_ESPNOW_EXIST) {
        Serial.print("[ESP-NOW] Failed to add peer: 0x");
        Serial.println(addResult, HEX);
        break;
      }
    }

    result = esp_now_send(peerMAC, (uint8_t *)&msg, sizeof(msg));
    if (result == ESP_OK) {
      break;
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND && sendAttempts == 0) {
      // Peer lost, try to re-register and retry once
      Serial.println("[ESP-NOW] Peer lost before send, retrying registration...");
      esp_now_del_peer(peerMAC);
      esp_now_peer_info_t peerInfo;
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, peerMAC, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      peerInfo.ifidx = WIFI_IF_STA;
      esp_now_add_peer(&peerInfo);
    } else {
      break;
    }
    sendAttempts++;
  }
  lastBroadcast = millis();

  if (result == ESP_OK) {
    Serial.print("TX: estado=");
    Serial.print(msg.state);
    Serial.print(", request=");
    Serial.print(msg.request);
    Serial.print(", dist=");
    Serial.println(msg.distance_cm);
  } else {
    Serial.print("Error TX ESP-NOW: 0x");
    Serial.println(result, HEX);
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
      return DEVICE_ID > 1;  // B tiene prioridad en empate (ID=2 > 1)
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
        // Solo pasar a verde si el otro NO está en ALL_RED
        if (remoteState != STATE_ALL_RED) {
          // Decidir quién pasa a verde
          if (shouldGetPriority() || (cycleCount % 2 == 1)) {  // B arranca en ciclos impares
            if (canTransitionToGreen()) {
              currentState = STATE_GREEN;
              greenDuration = vehicleDetected ? MAX_GREEN : GREEN_NORMAL;
              stateStartTime = millis();
              Serial.println("-> VERDE");
            }
          } else {
            currentState = STATE_RED;
            stateStartTime = millis();
            Serial.println("-> ROJO (turno de A)");
          }
        }
        // Si ambos siguen en ALL_RED, esperar hasta que uno avance
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
  Serial.println("\n=== SEMAFORO B - Iniciando ===");
  
  // Configurar pines
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  setLEDs(false, false, false);
  
  // Inicializar OLED (DESHABILITADO TEMPORALMENTE)
  /*
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error inicializando OLED");
  } else {
    Serial.println("OLED OK");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("SEMAFORO B");
    display.println("Iniciando...");
    display.display();
  }
  */
  Serial.println("OLED: Deshabilitado (no conectado)");
  
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
