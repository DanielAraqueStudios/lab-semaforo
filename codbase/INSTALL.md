# INSTALL.md — Instrucciones de compilación y carga (Arduino IDE)

Requisitos previos:
- Arduino IDE (preferible versión reciente) o Visual Studio Code + PlatformIO.
- Añadir soporte ESP32 en Board Manager (esp32 by Espressif):
  - URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
- Instalar librerías necesarias (Library Manager):
  - Adafruit SSD1306
  - Adafruit GFX

Configuración en Arduino IDE:
1. Ir a Tools -> Board -> Boards Manager -> instalar "esp32" (Espressif Systems).
2. Seleccionar la placa apropiada: "ESP32S3 Dev Module" o la que corresponda.
3. Configurar Upload Speed y Partition Scheme según tu placa (valores por defecto suelen funcionar).

Carga del sketch:
- El código de referencia se entregará en `codbase/src/traffic.ino` (o `traffic.ino`).
- Antes de compilar, ajustar `#define DEVICE_ID 1` o `2` según la placa (A o B).
- Configurar la dirección MAC del peer en el código si es necesario (se imprime al iniciar).

Depuración:
- Abrir Serial Monitor a 115200 bps.
- Revisar logs de ESP-NOW y mensajes recibidos.

Notas específicas:
- Asegurar que cada ESP tenga una identificación única (DEVICE_ID) en el sketch.
- Si usas HC-SR04, verifica que el pin ECHO esté protegido a 3.3V cuando sea necesario.

Problemas comunes:
- Error de compilación por librerías faltantes -> instalar Adafruit_SSD1306 y Adafruit_GFX.
- ESP-NOW requiere activar WiFi mode en código (WiFi.mode(WIFI_STA)).

Siguientes pasos:
- Ejecutar pruebas según `TEST_PLAN.md`.
- Ajustar umbrales y tiempos en `DESIGN.md` y en los defines del sketch.