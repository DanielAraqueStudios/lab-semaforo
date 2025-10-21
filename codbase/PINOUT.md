# PINOUT.md — Asignación de pines y diagrama de conexión

Ejemplo de asignación de pines para cada ESP (ajustar según la placa):

ESP_A (SEMAFORO A)
- GPIO 1  -> LED_ROJO_A (con resistencia 220Ω)
- GPIO 2  -> LED_AMARILLO_A
- GPIO 42 -> LED_VERDE_A
- GPIO 40 -> TRIG_HCSR04_A
- GPIO 39 -> ECHO_HCSR04_A (usar divisor si ECHO=5V)
- GPIO 21 -> OLED SDA
- GPIO 47 -> OLED SCL
- GPIO 38 -> BUZZER_A (opcional)

Pines disponibles para expansión: 35, 36, 37, 48

ESP_B (SEMAFORO B)
- GPIO 4  -> LED_ROJO_B (con resistencia 220Ω)
- GPIO 5  -> LED_AMARILLO_B
- GPIO 6  -> LED_VERDE_B
- GPIO 7  -> TRIG_HCSR04_B
- GPIO 15 -> ECHO_HCSR04_B (usar divisor si ECHO=5V)
- GPIO 17 -> OLED SDA
- GPIO 18 -> OLED SCL
- GPIO 16 -> BUZZER_B (opcional)

Pines disponibles para expansión B: 3, 8, 9, 10, 11, 12, 13, 14, 46

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