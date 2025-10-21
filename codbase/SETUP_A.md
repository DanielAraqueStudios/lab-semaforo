# CONFIGURACIÓN Y USO DE traffic_A.ino

## Archivo generado
`codbase/traffic_A.ino` — Código completo para SEMAFORO A (ESP32-S3)

## Características implementadas
✅ Control de 3 LEDs (Rojo, Amarillo, Verde)
✅ Lectura de sensor ultrasónico HC-SR04
✅ Comunicación ESP-NOW con SEMAFORO B
✅ Pantalla OLED I2C (SSD1306) con estado en tiempo real
✅ Máquina de estados con sincronización
✅ Lógica adaptativa (prioridad según detección de vehículos)
✅ Modo normal y modo adaptativo
✅ Interlock de seguridad (nunca verde simultáneo)
✅ Buzzer opcional para feedback
✅ Serial debug a 115200 bps

## Librerías necesarias (Arduino IDE)
Instalar desde Library Manager:
1. **Adafruit SSD1306** (by Adafruit)
2. **Adafruit GFX Library** (by Adafruit)

Las librerías WiFi y esp_now.h están incluidas en el core ESP32.

## Configuración ANTES de compilar

### 1. Dirección MAC del peer (IMPORTANTE)
En la línea 48 del código, encontrarás:
```cpp
uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // CAMBIAR ESTO
```

Debes reemplazar esto con la dirección MAC real del SEMAFORO B.

**Cómo obtener la MAC del otro ESP32:**
1. Carga un sketch simple en el ESP32-S3 B con:
   ```cpp
   void setup() {
     Serial.begin(115200);
     WiFi.mode(WIFI_STA);
     Serial.println(WiFi.macAddress());
   }
   ```
2. Anota la MAC (formato: AA:BB:CC:DD:EE:FF)
3. Convierte a formato hex array: `{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}`
4. Reemplaza en la línea 48

### 2. Verificar pines (opcional)
Si tu placa ESP32-S3 usa pines diferentes, ajusta en las líneas 33-43:
- LED_ROJO = 1
- LED_AMARILLO = 2
- LED_VERDE = 42
- TRIG_PIN = 40
- ECHO_PIN = 39
- BUZZER_PIN = 38
- OLED_SDA = 21
- OLED_SCL = 47

**NOTA:** Estos pines están optimizados para ESP32-S3. GPIO1 y GPIO2 son seguros en S3.

### 3. Ajustar parámetros (opcional)
Líneas 51-60:
- `GREEN_NORMAL` = tiempo en verde normal (10000 ms = 10 s)
- `MAX_GREEN` = tiempo máximo en verde adaptativo (20000 ms)
- `DETECTION_THRESHOLD_CM` = distancia para detectar vehículo (100 cm)

## Pasos para compilar y cargar

### En Arduino IDE:
1. Abrir `traffic_A.ino`
2. Ir a **Tools** → **Board** → **ESP32S3 Dev Module** (o tu placa específica)
3. Configurar:
   - Upload Speed: 115200
   - USB CDC On Boot: Enabled (si usas USB)
   - Flash Size: 4MB o según tu placa
   - Partition Scheme: Default
4. Conectar ESP32-S3 por USB
5. Seleccionar el puerto COM correcto en **Tools** → **Port**
6. Presionar el botón **Upload** (o Ctrl+U)
7. Esperar compilación y carga (puede tomar 1-2 minutos)

### Verificar funcionamiento:
1. Abrir **Serial Monitor** (Ctrl+Shift+M)
2. Configurar a **115200 baud**
3. Deberías ver:
   ```
   === SEMAFORO A - Iniciando ===
   MAC Address: XX:XX:XX:XX:XX:XX
   OLED OK
   Peer añadido correctamente
   === Sistema listo ===
   ```

## Cableado del hardware

### LEDs (con resistencias 220Ω):
- LED ROJO (ánodo) → GPIO 1 → resistencia → LED (cátodo) → GND
- LED AMARILLO → GPIO 2 → resistencia → GND
- LED VERDE → GPIO 42 → resistencia → GND

### Sensor HC-SR04:
- VCC → 5V (o VIN del ESP32)
- TRIG → GPIO 40
- ECHO → GPIO 39 (usar divisor de voltaje si ECHO=5V: 2 resistencias 1kΩ y 2kΩ)
- GND → GND

### OLED I2C:
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21
- SCL → GPIO 47

### Buzzer (opcional):
- Positivo → GPIO 38
- Negativo → GND
(Si requiere más corriente, usar transistor NPN)

## Información en la pantalla OLED
El display mostrará:
- **Línea 1:** "SEMAFORO A"
- **Línea 2:** Estado actual (VERDE, AMARILLO, ROJO, ALL RED)
- **Línea 3:** Tiempo restante en el estado actual (segundos)
- **Línea 4:** Detección de vehículo ("VEH: SI (XXcm)" o "VEH: NO")
- **Línea 5:** Estado del semáforo remoto B
- **Línea 6:** Estado de sincronización ("OK", "PRIO A", "PRIO B", "SIN SYNC")

## Depuración por Serial
Mensajes útiles:
- `TX: estado=X, request=X, dist=XXX` — Mensaje enviado por ESP-NOW
- `RX de ESP X: estado=X, request=X, dist=XXX` — Mensaje recibido
- `-> VERDE`, `-> AMARILLO`, `-> ROJO` — Cambios de estado
- `Sin comunicación con peer - modo seguro` — Pérdida de conexión

## Pruebas básicas
1. **Sin vehículos:** Verificar ciclo normal (10s verde → 3s amarillo → rojo)
2. **Con vehículo:** Poner objeto <100cm del sensor; verificar extensión de verde
3. **Sincronización:** Verificar que nunca ambos semáforos estén en verde
4. **OLED:** Confirmar actualización de información en pantalla

## Siguiente paso
Generar `traffic_B.ino` para el segundo ESP32-S3 con configuración simétrica.

## Problemas comunes

### "OLED not found"
- Verificar conexiones I2C
- Probar dirección 0x3D en vez de 0x3C (línea 256)
- Escanear I2C con sketch I2C Scanner

### "Error añadiendo peer"
- Verificar que la MAC del peer sea correcta
- Reiniciar ambos ESP32

### Sensor da 9999 cm
- Verificar conexiones TRIG/ECHO
- Verificar alimentación 5V del HC-SR04
- Reducir distancia o apuntar a superficie reflectante

### No compila
- Instalar librerías Adafruit_SSD1306 y Adafruit_GFX
- Verificar que el board ESP32 esté instalado
- Actualizar Arduino IDE a versión reciente

## Modificaciones para traffic_B.ino
Para crear el código del SEMAFORO B:
1. Cambiar `DEVICE_ID` de 1 a 2 (línea 46)
2. Cambiar `peerMAC[]` a la MAC del SEMAFORO A
3. Ajustar pines si es necesario (para evitar conflictos)
4. Cambiar "SEMAFORO A" por "SEMAFORO B" en displays/serial
