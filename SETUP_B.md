# CONFIGURACIÓN Y USO DE traffic_B.ino

## Archivo generado
`traffic_B.ino` — Código completo para SEMAFORO B (ESP32-S3) en la raíz del proyecto

## Diferencias clave respecto a traffic_A.ino
- **DEVICE_ID = 2** (identifica al SEMAFORO B)
- **Pines diferentes** para evitar conflictos si ambos ESP están en la misma protoboard
- **Texto en pantalla y serial** cambiado a "SEMAFORO B"
- **Lógica de alternancia** ajustada (B arranca en ciclos impares)
- **Peer MAC** debe apuntar al SEMAFORO A

## Librerías necesarias (Arduino IDE)
Instalar desde Library Manager:
1. **Adafruit SSD1306** (by Adafruit)
2. **Adafruit GFX Library** (by Adafruit)

## Configuración ANTES de compilar

### 1. Dirección MAC del SEMAFORO A (CRÍTICO)
En la línea 48 del código:
```cpp
uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // CAMBIAR ESTO
```

**Procedimiento:**
1. Primero carga `traffic_A.ino` en el primer ESP32
2. Abre Serial Monitor (115200 baud)
3. Anota la MAC que aparece: `MAC Address: AA:BB:CC:DD:EE:FF`
4. Convierte a formato array: `{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}`
5. Reemplaza en `traffic_B.ino` línea 48

### 2. Pines asignados (verificar antes de cablear)
```cpp
LED_ROJO = 4
LED_AMARILLO = 5
LED_VERDE = 6
TRIG_PIN = 7
ECHO_PIN = 15
BUZZER_PIN = 16
OLED_SDA = 17
OLED_SCL = 18
```

**NOTA:** Estos pines son completamente DIFERENTES a los de A para evitar conflictos y están optimizados para ESP32-S3.

### 3. Parámetros del sistema (iguales a A)
- `GREEN_NORMAL` = 10000 ms (10 segundos)
- `MAX_GREEN` = 20000 ms (20 segundos)
- `DETECTION_THRESHOLD_CM` = 100 cm
- `YELLOW_DURATION` = 3000 ms
- `ALL_RED_DURATION` = 1000 ms

## Pasos para compilar y cargar

### En Arduino IDE:
1. Abrir `traffic_B.ino` desde la raíz del proyecto
2. Ir a **Tools** → **Board** → **ESP32S3 Dev Module**
3. Configurar:
   - Upload Speed: 115200
   - USB CDC On Boot: Enabled
   - Flash Size: 4MB (o según tu placa)
   - Partition Scheme: Default
4. Conectar el **segundo ESP32-S3** por USB
5. Seleccionar el puerto COM correcto en **Tools** → **Port**
6. Presionar **Upload** (Ctrl+U)
7. Esperar compilación y carga

### Verificar funcionamiento:
1. Abrir **Serial Monitor** a 115200 baud
2. Deberías ver:
   ```
   === SEMAFORO B - Iniciando ===
   MAC Address: XX:XX:XX:XX:XX:XX
   OLED OK
   Peer añadido correctamente
   === Sistema listo ===
   ```

## Cableado del hardware (SEMAFORO B)

### LEDs (con resistencias 220Ω):
- LED ROJO → GPIO 4 → resistencia 220Ω → GND
- LED AMARILLO → GPIO 5 → resistencia 220Ω → GND
- LED VERDE → GPIO 6 → resistencia 220Ω → GND

### Sensor HC-SR04:
- VCC → 5V
- TRIG → GPIO 7
- ECHO → GPIO 15 (con divisor de voltaje si necesario)
- GND → GND

### OLED I2C:
- VCC → 3.3V
- GND → GND
- SDA → GPIO 17
- SCL → GPIO 18

### Buzzer (opcional):
- Positivo → GPIO 16
- Negativo → GND

## Información en la pantalla OLED
- **Línea 1:** "SEMAFORO B"
- **Línea 2:** Estado actual (VERDE, AMARILLO, ROJO, ALL RED)
- **Línea 3:** Tiempo restante (segundos)
- **Línea 4:** Detección de vehículo ("VEH: SI (XXcm)" o "VEH: NO")
- **Línea 5:** Estado del SEMAFORO A remoto
- **Línea 6:** Estado de sincronización ("OK", "PRIO B", "PRIO A", "SIN SYNC")

## Pruebas de integración (ambos ESP32)

### 1. Prueba básica de comunicación
1. Cargar `traffic_A.ino` en ESP A
2. Cargar `traffic_B.ino` en ESP B
3. Encender ambos
4. Verificar en Serial Monitor de ambos que reciben mensajes: `RX de ESP X`
5. Las pantallas OLED deben mostrar "OK" en sincronización

### 2. Prueba de ciclo normal (sin vehículos)
- Ambos semáforos deben alternar: uno en verde mientras el otro está en rojo
- Verificar tiempos: 10s verde → 3s amarillo → rojo
- **NUNCA** ambos en verde simultáneamente

### 3. Prueba de detección (carril B)
- Poner objeto < 100cm frente al sensor B
- Verificar pantalla B: "VEH: SI"
- SEMAFORO B debe solicitar prioridad: "PRIO B"
- En el siguiente ciclo, B debe obtener verde o extender su verde

### 4. Prueba de conflicto (ambos carriles con vehículos)
- Objetos < 100cm en ambos sensores
- Sistema debe resolver por distancia: el vehículo más cercano gana
- Solo un semáforo en verde a la vez

### 5. Prueba de pérdida de comunicación
- Apagar uno de los ESP temporalmente
- El otro debe mostrar "SIN SYNC"
- Al reconectar, deben resincronizar automáticamente

### 6. Prueba de estabilidad
- Dejar sistema corriendo durante 30 minutos
- Verificar: sin bloqueos, sin cambios erráticos
- Pantallas actualizadas continuamente

## Mensajes Serial importantes
- `TX: estado=X, request=X, dist=XXX` — Transmisión ESP-NOW
- `RX de ESP 1: estado=X...` — Recepción desde SEMAFORO A
- `-> VERDE`, `-> AMARILLO`, `-> ROJO` — Transiciones de estado
- `Sin comunicación con peer - modo seguro` — Alerta de desconexión

## Comparación de configuraciones A vs B

| Parámetro | SEMAFORO A | SEMAFORO B |
|-----------|------------|------------|
| DEVICE_ID | 1 | 2 |
| LED_ROJO | GPIO 1 | GPIO 4 |
| LED_AMARILLO | GPIO 2 | GPIO 5 |
| LED_VERDE | GPIO 42 | GPIO 6 |
| TRIG_PIN | GPIO 40 | GPIO 7 |
| ECHO_PIN | GPIO 39 | GPIO 15 |
| BUZZER_PIN | GPIO 38 | GPIO 16 |
| OLED_SDA | GPIO 21 | GPIO 17 |
| OLED_SCL | GPIO 47 | GPIO 18 |
| Peer MAC | MAC de B | MAC de A |
| Ciclo inicial | Par (0, 2, 4...) | Impar (1, 3, 5...) |

## Solución de problemas

### "Peer añadido correctamente" pero no recibe mensajes
- Verificar que la MAC del peer sea correcta
- Reiniciar ambos ESP32 simultáneamente
- Verificar que ambos estén en modo WIFI_STA

### Ambos semáforos se quedan en rojo
- Problema de sincronización inicial
- Solución: reiniciar ambos ESP32 al mismo tiempo
- Verificar comunicación bidireccional en Serial Monitor

### Sensor lee 9999 cm constantemente
- Verificar conexiones TRIG/ECHO
- Verificar alimentación 5V del HC-SR04
- Poner objeto reflectante frente al sensor para probar

### OLED no muestra nada
- Verificar conexiones I2C (SDA/SCL)
- Probar cambiar dirección I2C de 0x3C a 0x3D (línea 471)
- Usar I2C Scanner para detectar dirección correcta

## Archivos relacionados
- `traffic_A.ino` — Código para primer ESP32 (en raíz o codbase/)
- `codbase/DESIGN.md` — Diseño del sistema
- `codbase/PROTOCOL.md` — Protocolo ESP-NOW
- `codbase/TEST_PLAN.md` — Plan de pruebas completo

## Siguiente paso
Una vez ambos ESP32 estén configurados y cargados:
1. Encender ambos simultáneamente
2. Abrir Serial Monitor de ambos (en dos instancias de Arduino IDE)
3. Observar intercambio de mensajes
4. Verificar alternancia correcta de semáforos
5. Realizar pruebas con sensores según `TEST_PLAN.md`
