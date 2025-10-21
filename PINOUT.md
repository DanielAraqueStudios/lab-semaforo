# 📌 PINOUT - Configuración de Pines ESP32-S3

## 🔧 ESP32-S3 A (SEMAFORO_A)

### 📍 Identificación
- **MAC Address**: `10:51:DB:82:5D:70`
- **Device ID**: `1`
- **Peer MAC** (ESP B): `50:78:7D:15:B3:84`

### 🔴 LEDs de Semáforo
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **1**    | LED_ROJO | Luz roja del semáforo (con resistencia 220Ω) |
| **2**    | LED_AMARILLO | Luz amarilla del semáforo (con resistencia 220Ω) |
| **42**   | LED_VERDE | Luz verde del semáforo (con resistencia 220Ω) |

### 📡 Sensor Ultrasónico HC-SR04
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **40**   | TRIG_PIN | Pin de disparo del sensor |
| **39**   | ECHO_PIN | Pin de recepción de eco |

### 🔊 Buzzer (Opcional)
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **38**   | BUZZER_PIN | Señal sonora (alerta cambio de estado) |

### 🖥️ Pantalla OLED I2C (Deshabilitada)
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **21**   | OLED_SDA | Datos I2C (SSD1306 - 0x3C) |
| **47**   | OLED_SCL | Reloj I2C |

> ⚠️ **OLED actualmente deshabilitada** en el código para evitar errores I2C NACK

---

## 🔧 ESP32-S3 B (SEMAFORO_B)

### 📍 Identificación
- **MAC Address**: `50:78:7D:15:B3:84`
- **Device ID**: `2`
- **Peer MAC** (ESP A): `10:51:DB:82:5D:70`

### 🔴 LEDs de Semáforo
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **4**    | LED_ROJO | Luz roja del semáforo (con resistencia 220Ω) |
| **5**    | LED_AMARILLO | Luz amarilla del semáforo (con resistencia 220Ω) |
| **6**    | LED_VERDE | Luz verde del semáforo (con resistencia 220Ω) |

### 📡 Sensor Ultrasónico HC-SR04
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **7**    | TRIG_PIN | Pin de disparo del sensor |
| **15**   | ECHO_PIN | Pin de recepción de eco |

### 🔊 Buzzer (Opcional)
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **16**   | BUZZER_PIN | Señal sonora (alerta cambio de estado) |

### 🖥️ Pantalla OLED I2C (Deshabilitada)
| Pin GPIO | Componente | Descripción |
|----------|------------|-------------|
| **17**   | OLED_SDA | Datos I2C (SSD1306 - 0x3C) |
| **18**   | OLED_SCL | Reloj I2C |

> ⚠️ **OLED actualmente deshabilitada** en el código para evitar errores I2C NACK

---

## 🔌 Conexiones Físicas Recomendadas

### LED (Rojo, Amarillo, Verde)
```
ESP32 GPIO → Resistencia 220Ω → LED (+) → LED (-) → GND
```

### HC-SR04
```
ESP32 TRIG → HC-SR04 Trig
ESP32 ECHO → HC-SR04 Echo
5V → HC-SR04 VCC
GND → HC-SR04 GND
```

### OLED SSD1306 (cuando se habilite)
```
ESP32 SDA → OLED SDA
ESP32 SCL → OLED SCL
3.3V → OLED VCC
GND → OLED GND
```

### Buzzer Pasivo
```
ESP32 GPIO → Buzzer (+)
GND → Buzzer (-)
```

---

## 📊 Resumen de Asignación

| Componente | ESP A (Pines) | ESP B (Pines) |
|------------|---------------|---------------|
| **LED Rojo** | GPIO 1 | GPIO 4 |
| **LED Amarillo** | GPIO 2 | GPIO 5 |
| **LED Verde** | GPIO 42 | GPIO 6 |
| **TRIG** | GPIO 40 | GPIO 7 |
| **ECHO** | GPIO 39 | GPIO 15 |
| **BUZZER** | GPIO 38 | GPIO 16 |
| **OLED SDA** | GPIO 21 | GPIO 17 |
| **OLED SCL** | GPIO 47 | GPIO 18 |

---

## 🔄 Comunicación ESP-NOW

### Configuración Actual
- **ESP A** envía a: `50:78:7D:15:B3:84` (MAC de ESP B)
- **ESP B** envía a: `10:51:DB:82:5D:70` (MAC de ESP A)
- **Canal WiFi**: 0 (auto)
- **Encriptación**: Deshabilitada
- **Intervalo broadcast**: 200ms

### Estructura del Mensaje
```cpp
struct TrafficMsg {
  uint8_t sender_id;      // 1=ESP_A, 2=ESP_B
  uint8_t seq;            // Contador de secuencia
  uint8_t state;          // Estado actual (0-4)
  uint8_t request;        // Solicitud de prioridad
  uint16_t distance_cm;   // Distancia HC-SR04
  uint32_t timestamp_ms;  // Timestamp local
};
```

---

## 🛠️ Notas de Hardware

### ⚡ Alimentación
- **ESP32-S3**: 3.3V lógica, 5V USB
- **LEDs**: 220Ω protegen a ~15mA cada uno
- **HC-SR04**: Requiere 5V para funcionar correctamente
- **OLED**: 3.3V I2C compatible

### ⚠️ Precauciones
1. **No conectar LEDs sin resistencias** - daño permanente al ESP32
2. **HC-SR04 necesita 5V** - no funciona correctamente con 3.3V
3. **I2C pull-ups**: OLED SSD1306 tiene resistencias integradas
4. **Pines solo entrada**: No usar GPIOs 19, 20 (USB)

### 🔍 Testing
- Use `TEST_MAC.ino` para obtener direcciones MAC
- Verifique conexiones I2C con `Wire.begin()` y scanner
- Pruebe LEDs individualmente antes de cargar código completo

---

## 📚 Referencias
- [ESP32-S3 Pinout](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html)
- [HC-SR04 Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
- [SSD1306 OLED](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
