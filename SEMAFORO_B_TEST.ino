/*
 * SEMAFOOR_B_TEST.ino
 * Sketch de prueba para ESP32-S3 (SEMAFORO B)
 * - Mide HC-SR04
 * - Controla 3 LEDs (Rojo, Amarillo, Verde)
 * - Buzzer opcional
 * - Envía lecturas y estado por UART (115200)
 * - Acepta comandos sencillos por UART para forzar estado o alternar modo auto/manual
 *
 * Pines configurados para ESP B:
 * LED_ROJO = 4
 * LED_AMARILLO = 5
 * LED_VERDE = 6
 * TRIG_PIN = 7
 * ECHO_PIN = 15
 * BUZZER_PIN = 16 (opcional)
 * OLED_SDA = 17, OLED_SCL = 18 (si se conecta)
 */

#include <Arduino.h>

// Pines (ajustar si es necesario)
#define LED_ROJO     4
#define LED_AMARILLO 5
#define LED_VERDE    6
#define TRIG_PIN     7
#define ECHO_PIN     15
#define BUZZER_PIN   16

#define DETECTION_THRESHOLD_CM 5
#define DETECTION_PERSIST_MS   500
#define DEFAULT_GREEN_MS       10000
#define YELLOW_MS              3000
#define ALL_RED_MS             1000
#define SERIAL_BAUD            115200
#define REPORT_INTERVAL_MS     500

enum State { S_ALL_RED=0, S_GREEN=1, S_YELLOW=2, S_RED=3 };

State currentState = S_ALL_RED;
State prevState = S_ALL_RED;
unsigned long stateStart = 0;
unsigned long lastReport = 0;

// Detección
bool vehicleDetected = false;
unsigned long lastDetectionTime = 0;
uint16_t lastDistance = 9999;

// Debug control
bool DEBUG_ENABLED = false; // set true to enable debug prints

// Modo manual/auto
bool autoMode = true;

uint32_t seq = 0;

// Funciones de LEDs
void setLEDs(bool r, bool y, bool g) {
  digitalWrite(LED_ROJO, r ? HIGH : LOW);
  digitalWrite(LED_AMARILLO, y ? HIGH : LOW);
  digitalWrite(LED_VERDE, g ? HIGH : LOW);
}

void updateLEDs() {
  switch(currentState) {
    case S_ALL_RED: setLEDs(true, false, false); break;
    case S_GREEN: setLEDs(false, false, true); break;
    case S_YELLOW: setLEDs(false, true, false); break;
    case S_RED: setLEDs(true, false, false); break;
  }
}

// Medición HC-SR04
uint16_t measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  if (duration == 0) return 9999;
  uint16_t dist = (uint16_t)(duration * 0.034 / 2);
  return dist;
}

// Lectura y lógica simple: una lectura <= umbral activa la ventana de detección
void updateDetection() {
  uint16_t d = measureDistance();
  lastDistance = d;
  if (d <= DETECTION_THRESHOLD_CM) {
    lastDetectionTime = millis();
    if (DEBUG_ENABLED) {
      Serial.print("DEBUG: detection reading d="); Serial.println(d);
      Serial.print("DEBUG: lastDetectionTime="); Serial.println(lastDetectionTime);
    }
  }

  // vehicleDetected true durante DETECTION_PERSIST_MS desde la última lectura válida
  if (millis() - lastDetectionTime <= DETECTION_PERSIST_MS) {
    if (!vehicleDetected) {
      vehicleDetected = true;
      if (DEBUG_ENABLED) {
        Serial.print("DEBUG: vehicleDetected=1 at "); Serial.println(millis());
      }
    }
  } else {
    if (vehicleDetected) {
      if (DEBUG_ENABLED) {
        Serial.print("DEBUG: vehicleDetected reset at "); Serial.println(millis());
      }
    }
    vehicleDetected = false;
  }
}

void transitionStateMachine() {
  unsigned long elapsed = millis() - stateStart;
  switch(currentState) {
    case S_ALL_RED:
      if (elapsed >= ALL_RED_MS) {
        currentState = S_GREEN; stateStart = millis();
        Serial.println("-> VERDE");
      }
      break;
    case S_GREEN:
      if (autoMode) {
        if (vehicleDetected && elapsed < DEFAULT_GREEN_MS) {
          // opcional: extender verde
        }
        if (elapsed >= DEFAULT_GREEN_MS) {
          currentState = S_YELLOW; stateStart = millis();
          Serial.println("-> AMARILLO");
        }
      }
      break;
    case S_YELLOW:
      if (elapsed >= YELLOW_MS) { currentState = S_RED; stateStart = millis(); Serial.println("-> ROJO"); }
      break;
    case S_RED:
      if (elapsed >= DEFAULT_GREEN_MS/2) { currentState = S_ALL_RED; stateStart = millis(); Serial.println("-> ALL_RED"); }
      break;
  }
  updateLEDs();
}

// Serial commands: 'g' = green, 'r' = red, 'a' = auto mode on, 'm' = manual mode, 's' = status, 'd' debug toggle
void processSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') continue;
    switch (c) {
      case 'g': currentState = S_GREEN; stateStart = millis(); Serial.println("CMD: FORZADO VERDE"); break;
      case 'r': currentState = S_RED; stateStart = millis(); Serial.println("CMD: FORZADO ROJO"); break;
      case 'y': currentState = S_YELLOW; stateStart = millis(); Serial.println("CMD: FORZADO AMARILLO"); break;
      case 'a': autoMode = true; Serial.println("CMD: MODO AUTO ON"); break;
      case 'm': autoMode = false; Serial.println("CMD: MODO MANUAL ON"); break;
      case 't': autoMode = !autoMode; Serial.print("CMD: TOGGLE AUTO -> "); Serial.println(autoMode ? "AUTO" : "MANUAL"); break;
      case 'd': DEBUG_ENABLED = !DEBUG_ENABLED; Serial.print("CMD: DEBUG -> "); Serial.println(DEBUG_ENABLED ? "ON" : "OFF"); break;
      case 's': {
        Serial.print("STATUS: ");
        Serial.print("state="); Serial.print((int)currentState);
        Serial.print(", dist="); Serial.print(lastDistance);
        Serial.print(", veh="); Serial.print(vehicleDetected ? "1" : "0");
        Serial.print(", auto="); Serial.println(autoMode ? "1" : "0");
        break;
      }
      default:
        Serial.print("CMD: Desconocido -> "); Serial.println(c);
    }
  }
}

void printMenu() {
  Serial.println("---- SEMAforo B - TEST UART ----");
  Serial.println("Comandos:");
  Serial.println("  g -> Forzar VERDE");
  Serial.println("  r -> Forzar ROJO");
  Serial.println("  y -> Forzar AMARILLO");
  Serial.println("  a -> Modo AUTO ON");
  Serial.println("  m -> Modo MANUAL ON");
  Serial.println("  t -> Toggle AUTO/MANUAL");
  Serial.println("  s -> Mostrar STATUS actual");
  Serial.println("  d -> Toggle DEBUG prints");
  Serial.println("\nNota: vehicleDetected = TRUE cuando distancia <= 5 cm (umbral actual)");
  Serial.println("--------------------------------");
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(100);
  Serial.println("\nSEMAFORO_B_TEST - Iniciando...");

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLEDs(false, false, false);
  currentState = S_ALL_RED;
  stateStart = millis();

  printMenu();
}

void loop() {
  processSerial();
  updateDetection();
  if (autoMode) transitionStateMachine();
  updateLEDs();

  // Reportar por UART periódicamente
  if (millis() - lastReport >= REPORT_INTERVAL_MS) {
    lastReport = millis();
    // Formato CSV: seq,ts,state,dist,veh,auto
    Serial.print(seq++);
    Serial.print(",");
    Serial.print(millis());
    Serial.print(",");
    Serial.print((int)currentState);
    Serial.print(",");
    Serial.print(lastDistance);
    Serial.print(",");
    Serial.print(vehicleDetected ? "1" : "0");
    Serial.print(",");
    Serial.println(autoMode ? "1" : "0");
  }

  delay(10);
}
