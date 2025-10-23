/*
 * Don Cangrejo (Esclavo, ESP32-WROOM)
 * Recibe código y cantidad por ESP-NOW
 * Enciende LED GPIO 2 si la cantidad no corresponde
 * MAC de este ESP: 14:33:5C:38:D0:0C
 */

#include <WiFi.h>
#include <esp_now.h>

#define LED_ALERTA 2

struct __attribute__((packed)) ProductoMsg {
  char codigo[8];
  uint8_t cantidad;
};

// Tabla de cantidades esperadas
struct Stock {
  const char* codigo;
  uint8_t cantidad;
};

Stock inventario[] = {
  {"HBRGR", 50},
  {"SWBRD", 50},
  {"SHRMP", 15},
  {"PICKL", 20},
  {"TMATO", 20},
  {"LETCE", 20}
};
const int nItems = sizeof(inventario)/sizeof(Stock);

void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len != sizeof(ProductoMsg)) return;
  ProductoMsg msg;
  memcpy(&msg, data, sizeof(msg));
  Serial.print("Recibido: "); Serial.print(msg.codigo); Serial.print(" "); Serial.println(msg.cantidad);
  bool ok = false;
  for (int i = 0; i < nItems; i++) {
    if (strncmp(msg.codigo, inventario[i].codigo, 5) == 0) {
      if (msg.cantidad == inventario[i].cantidad) {
        ok = true;
      }
      break;
    }
  }
  digitalWrite(LED_ALERTA, ok ? LOW : HIGH);
  if (!ok) Serial.println("ALERTA: Cantidad incorrecta!");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_ALERTA, OUTPUT);
  digitalWrite(LED_ALERTA, LOW);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    while (true);
  }
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("Esperando productos por ESP-NOW...");
}

void loop() {
  delay(10);
}
