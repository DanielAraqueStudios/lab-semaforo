# PINOUT.md — Asignación de pines y diagrama de conexión

Ejemplo de asignación de pines para cada ESP (ajustar según la placa):

ESP_A (SEMAFORO A)
- GPIO 16 -> LED_ROJO_A (con resistencia 220Ω)
- GPIO 17 -> LED_AMARILLO_A
- GPIO 18 -> LED_VERDE_A
- GPIO 25 -> TRIG_HCSR04_A
- GPIO 26 -> ECHO_HCSR04_A (usar divisor si ECHO=5V)
- GPIO 21 -> OLED SDA
- GPIO 22 -> OLED SCL
- GPIO 27 -> BUZZER_A (opcional)

ESP_B (SEMAFORO B)
- GPIO 2  -> LED_ROJO_B
- GPIO 4  -> LED_AMARILLO_B
- GPIO 5  -> LED_VERDE_B
- GPIO 32 -> TRIG_HCSR04_B
- GPIO 33 -> ECHO_HCSR04_B
- GPIO 21 -> OLED SDA
- GPIO 22 -> OLED SCL
- GPIO 15 -> BUZZER_B (opcional)

Conexiones generales:
- GND comunes entre ambos ESP y sensores.
- LEDs -> resistencias 220Ω -> pines GPIO.
- HC-SR04: Vcc 5V, TRIG a GPIO, ECHO a GPIO (usar divisor ó nivel lógico si necesario).
- OLED: VCC 3.3V, GND, SDA y SCL.

Diagrama lógico (texto):
- Cada ESP controla su conjunto de LEDs y su sensor. Las dos placas no comparten
  pines físicos, solo comunicación inalámbrica.

Notas:
- Verificar pines reservados de la placa ESP32-S3 antes de usar.
- Ajustar pines si usas módulos específicos (p. ej. WROOM, S3 devkit).