/*
 * IDENTIFY_MAC.ino
 * Sketch simple para identificar la MAC del ESP32-S3
 * Sube este sketch a cada ESP para saber su MAC
 */

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  delay(100);
  
  Serial.println("\n\n====================================");
  Serial.println("    IDENTIFICADOR DE MAC ESP32");
  Serial.println("====================================");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("====================================");
  Serial.println();
  
  // Identificación por MAC conocida
  String mac = WiFi.macAddress();
  if (mac == "10:51:DB:82:5D:70") {
    Serial.println(">>> ESTE ES EL ESP A <<<");
    Serial.println("Sube: traffic_A.ino");
  } else if (mac == "50:78:7D:15:B3:84") {
    Serial.println(">>> ESTE ES EL ESP B <<<");
    Serial.println("Sube: traffic_B.ino");
  } else {
    Serial.println(">>> ESP DESCONOCIDO <<<");
    Serial.println("Verifica las MACs configuradas");
  }
  Serial.println("====================================\n");

  // Configurar LED integrado (GPIO 2 en ESP32)
  pinMode(2, OUTPUT);
}

void loop() {
  // Parpadear LED integrado cada segundo (GPIO 2 en ESP32)
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
}
