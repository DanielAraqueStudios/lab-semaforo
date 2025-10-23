/*
 * Calamardo (Maestro, ESP32-S3)
 * Envia código y cantidad por Serial y ESP-NOW
 * MAC del esclavo (Don Cangrejo): 14:33:5C:38:D0:0C
 */

#include <WiFi.h>
#include <esp_now.h>

// MAC del receptor (Don Cangrejo)
uint8_t peerMAC[6] = {0x14, 0x33, 0x5C, 0x38, 0xD0, 0x0C};

struct __attribute__((packed)) ProductoMsg {
  char codigo[8];
  uint8_t cantidad;
};

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    while (true);
  }
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  Serial.println("Listo para enviar. Formato: CODIGO + cantidad (ej: HBRGR25)");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() < 6) {
      Serial.println("Formato inválido");
      return;
    }
    String codigo = input.substring(0, 5);
    String cantidadStr = input.substring(5);
    int cantidad = cantidadStr.toInt();
    if (cantidad <= 0 || cantidad > 255) {
      Serial.println("Cantidad inválida");
      return;
    }
    ProductoMsg msg = {};
    codigo.toCharArray(msg.codigo, 8);
    msg.cantidad = (uint8_t)cantidad;
    esp_err_t result = esp_now_send(peerMAC, (uint8_t*)&msg, sizeof(msg));
    if (result == ESP_OK) {
      Serial.print("Enviado: "); Serial.print(msg.codigo); Serial.print(" "); Serial.println(msg.cantidad);
    } else {
      Serial.println("Error enviando por ESP-NOW");
    }
  }
}
