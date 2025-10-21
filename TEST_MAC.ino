/*
 * TEST_MAC - Obtener dirección MAC del ESP32-S3
 * 
 * Este sketch simplemente imprime la MAC address del ESP32
 * sin inicializar periféricos (OLED, sensores, etc.)
 * 
 * INSTRUCCIONES:
 * 1. Subir este código al ESP32-S3
 * 2. Abrir Serial Monitor (115200 baud)
 * 3. COPIAR la dirección MAC que aparece
 * 4. Repetir con el segundo ESP32
 * 5. Intercambiar MACs en traffic_A.ino y traffic_B.ino
 */

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n===========================================");
  Serial.println("    TEST DE DIRECCIÓN MAC - ESP32-S3");
  Serial.println("===========================================\n");
  
  // Inicializar WiFi en modo estación
  WiFi.mode(WIFI_STA);
  delay(100);
  
  // Obtener y mostrar MAC
  String macStr = WiFi.macAddress();
  Serial.print("✅ MAC Address: ");
  Serial.println(macStr);
  Serial.println();
  
  // Convertir a formato de array C
  Serial.println("📋 Formato para código Arduino:");
  Serial.print("   uint8_t peerMAC[] = {");
  
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for(int i = 0; i < 6; i++) {
    Serial.print("0x");
    if(mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if(i < 5) Serial.print(", ");
  }
  Serial.println("};");
  Serial.println();
  
  Serial.println("===========================================");
  Serial.println("⚠️  IMPORTANTE:");
  Serial.println("   - COPIAR la MAC de este ESP32");
  Serial.println("   - PEGAR en el peerMAC[] del OTRO ESP32");
  Serial.println("   - Ejemplo:");
  Serial.println("     • Si este es ESP A → pegar en traffic_B.ino");
  Serial.println("     • Si este es ESP B → pegar en traffic_A.ino");
  Serial.println("===========================================\n");
}

void loop() {
  // Repetir cada 5 segundos para facilitar copia
  delay(5000);
  
  String macStr = WiFi.macAddress();
  Serial.print("MAC: ");
  Serial.print(macStr);
  Serial.println(" (presiona Reset para ver formato completo)");
}
