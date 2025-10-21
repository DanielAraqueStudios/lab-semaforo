# 🚦 Semáforo Inteligente con ESP32 y ESP-NOW

Proyecto académico — Universidad Militar Nueva Granada  
Curso: Bluetooth y NOW

---

## 🚀 INICIO RÁPIDO (5 PASOS)

### ¿Primera vez? Lee primero: **[`INICIO_RAPIDO.md`](INICIO_RAPIDO.md)**

```
1. 📡 Obtener MACs → Ejecutar TEST_MAC.ino
2. ✏️  Editar línea 48 en traffic_A.ino y traffic_B.ino
3. 🔌 Conectar LEDs (mínimo 3 por ESP32)
4. 📤 Subir código a ambos ESP32
5. ✅ Verificar comunicación ESP-NOW
```

**Tiempo estimado:** 15-30 minutos

---

## ⚠️ IMPORTANTE: Solución de problemas detectados

### 🔴 ¿Ves errores I2C NACK?
```
E (7813) i2c.master: I2C hardware NACK detected
```
👉 **Lee:** [`SOLUCION_OLED.md`](SOLUCION_OLED.md) — OLED deshabilitado temporalmente

### 🔴 ¿ESP32 recibe sus propios mensajes?
```
[A] RX de ESP 1: estado=3, ...  ← ❌ ESP A recibe de sí mismo
```
👉 **Lee:** [`CONFIGURACION_MAC.md`](CONFIGURACION_MAC.md) — Configurar MACs correctamente

### 🔴 ¿Logs difíciles de entender?
👉 **Lee:** [`ANALISIS_LOGS.md`](ANALISIS_LOGS.md) — Interpretación de mensajes

---

## 📖 Descripción

Sistema de control semafórico inteligente para intersección de dos vías con **ESP32-S3**, usando **ESP-NOW** para comunicación inalámbrica y **HC-SR04** para detección de vehículos.

### ✨ Características:
- **Adaptativo:** Prioridad al carril con vehículo más cercano
- **Sincronizado:** Comunicación ESP-NOW bidireccional (200ms)
- **Seguro:** Modo fail-safe si se pierde comunicación
- **Monitoreable:** Monitor Python PyQt6 con visualización en tiempo real
- **Extensible:** Sensor HC-SR04 extiende verde hasta 20s

## 📁 Estructura del repositorio

```
lab-semaforo/
├── 🚀 INICIO_RAPIDO.md          # ⭐ COMIENZA AQUÍ (setup en 5 pasos)
├── 🔧 CONFIGURACION_MAC.md      # Configurar direcciones ESP-NOW
├── 🖥️  SOLUCION_OLED.md          # Solucionar errores I2C NACK
├── 🔍 ANALISIS_LOGS.md          # Interpretar logs Serial
├── 📋 RESUMEN_CAMBIOS.md        # Historial de modificaciones
│
├── traffic_A.ino                # Código ESP32-S3 A (DEVICE_ID=1)
├── traffic_B.ino                # Código ESP32-S3 B (DEVICE_ID=2)
├── TEST_MAC.ino                 # Obtener dirección MAC rápidamente
│
├── monitor_semaforos.py         # 🐍 GUI PyQt6 de monitoreo
├── requirements.txt             # Dependencias Python
├── MONITOR_README.md            # Manual del monitor Python
│
├── SETUP_B.md                   # Setup específico ESP32 B
└── codbase/                     # 📚 Documentación técnica
    ├── README.md                # Índice de documentos
    ├── DESIGN.md                # Arquitectura y máquina de estados
    ├── PROTOCOL.md              # Protocolo ESP-NOW (10 bytes)
    ├── PINOUT.md                # GPIO de ambos ESP32
    ├── TEST_PLAN.md             # Plan de pruebas
    ├── INSTALL.md               # Instrucciones de compilación
    └── SETUP_A.md               # Setup específico ESP32 A
```

---

## 🔌 Componentes principales

| Componente | Cantidad | Notas |
|------------|----------|-------|
| ESP32-S3 DevKit | 2 | Soporte IDF v5.x, WiFi 2.4GHz |
| LEDs (R/Y/G) + resistencias 220Ω | 6 + 6 | GPIO 1,2,42 (A) / 4,5,6 (B) |
| Sensor HC-SR04 ultrasónico | 2 | Con divisor de voltaje ECHO (5V→3.3V) |
| Pantalla OLED I2C SSD1306 | 2 | **Opcional** (actualmente deshabilitadas) |
| Buzzer piezo | 2 | Opcional, alertas sonoras |
| Protoboard + cables | - | Conexiones |
| Fuente 5V/3.3V | 1 | USB o externa |

---

## 🎯 Estados del sistema

### Ciclo normal (sin vehículos):
```
┌─────────────┐
│  ALL_RED    │ ← Ambos en rojo (1s seguridad)
└──────┬──────┘
       ↓
┌─────────────┬─────────────┐
│  A: VERDE   │  B: ROJO    │ ← A pasa (10s)
└──────┬──────┴─────────────┘
       ↓
┌─────────────┬─────────────┐
│ A: AMARILLO │  B: ROJO    │ ← Transición (3s)
└──────┬──────┴─────────────┘
       ↓
┌─────────────┐
│  ALL_RED    │ ← Ambos en rojo (1s)
└──────┬──────┘
       ↓
┌─────────────┬─────────────┐
│  A: ROJO    │  B: VERDE   │ ← B pasa (10s)
└─────────────┴──────┬──────┘
       ↓             ↓
       (Ciclo se repite)
```

---

## 🧠 Lógica del semáforo (Explicación detallada)

### 📋 Estados posibles:

El sistema utiliza una máquina de estados con 5 estados principales:

```cpp
enum State {
  ALL_RED,   // 0: Ambos semáforos en rojo (seguridad)
  GREEN,     // 1: Verde (permite paso)
  YELLOW,    // 2: Amarillo (advertencia/transición)
  RED,       // 3: Rojo (detención)
  WAIT       // 4: Esperando turno
};
```

### 🔄 Flujo de estados:

#### **Caso 1: Sin vehículos detectados**

El sistema opera en un ciclo básico alternando entre ambos semáforos:

1. **ALL_RED (1s)** → Período de seguridad donde ambos están en rojo
2. **Semáforo A → GREEN (10s)** / Semáforo B → RED
3. **Semáforo A → YELLOW (3s)** / Semáforo B → RED (transición)
4. **ALL_RED (1s)** → Nuevo período de seguridad
5. **Semáforo A → RED** / Semáforo B → GREEN (10s)
6. **Semáforo A → RED** / Semáforo B → YELLOW (3s)
7. **Volver al paso 1**

**Duración del ciclo completo:** ~28 segundos (sin vehículos)

#### **Caso 2: Con vehículos detectados**

Cuando un vehículo es detectado (distancia ≤ 5 cm durante ≥ 500 ms):

**Verde extendido:**
- Si el semáforo está en VERDE y detecta vehículo → Extiende hasta **20s máximo**
- El tiempo se extiende mientras haya presencia continua
- Evita cortar el paso a vehículos en movimiento

**Solicitud de prioridad:**
- El semáforo en ROJO con vehículo detectado envía `request = 1`
- Esto notifica al otro semáforo que hay tráfico esperando
- El semáforo en verde puede acortar su ciclo si no tiene vehículos

### 🎯 Sistema de prioridad inteligente:

Cuando **AMBOS** semáforos detectan vehículos simultáneamente:

#### **1. Comparación de distancia:**
```cpp
if (myDistance < peerDistance) {
  // Mi vehículo está más cerca → Yo tengo prioridad
  shouldHaveGreen = true;
}
else if (peerDistance < myDistance) {
  // Vehículo del otro semáforo más cerca → Él tiene prioridad
  shouldHaveGreen = false;
}
```

**Ejemplo:**
- Semáforo A detecta vehículo a 3 cm
- Semáforo B detecta vehículo a 4 cm
- **Resultado:** A obtiene luz verde (vehículo más cercano)

#### **2. Desempate por ID:**
Si ambos vehículos están exactamente a la misma distancia:

```cpp
else if (peerDistance == myDistance) {
  // Empate → Gana el semáforo con ID mayor
  shouldHaveGreen = (DEVICE_ID > peerSenderId);
}
```

**Ejemplo:**
- Semáforo A (ID=1) detecta vehículo a 5 cm
- Semáforo B (ID=2) detecta vehículo a 5 cm
- **Resultado:** B obtiene luz verde (ID mayor)

### 🚗 Detección de vehículos (HC-SR04):

#### **Parámetros de detección:**
```cpp
#define DETECTION_THRESHOLD_CM 5      // Distancia mínima para detección
#define DETECTION_PERSIST_MS 500      // Tiempo mínimo de presencia
```

#### **Lógica de ventana temporal:**
```cpp
void updateVehicleDetection() {
  long distance = measureDistance();
  
  // Si detecta vehículo cercano → Actualizar timestamp
  if (distance <= DETECTION_THRESHOLD_CM) {
    lastDetectionTime = millis();
  }
  
  // Vehículo detectado SI la última lectura válida fue hace < 500ms
  vehicleDetected = (millis() - lastDetectionTime <= DETECTION_PERSIST_MS);
}
```

**¿Por qué esta lógica?**
- ✅ Evita falsos positivos por rebotes del ultrasonido
- ✅ No requiere lecturas continuas (más robusto)
- ✅ Permite detectar vehículos en movimiento lento
- ✅ Ventana de 500ms suficiente para actualizar estado

### 📡 Comunicación ESP-NOW:

Cada **200 ms**, ambos semáforos transmiten:

```cpp
struct TrafficMsg {
  uint8_t sender_id;      // 1 o 2 (identifica quién envía)
  uint8_t seq;            // Número de secuencia (0-255)
  uint8_t state;          // Estado actual (0-4)
  uint8_t request;        // 1 si tiene vehículo y pide prioridad
  uint16_t distance_cm;   // Distancia medida (9999 = sin detección)
  uint32_t timestamp_ms;  // Tiempo interno del ESP
};
```

**Protocolo de sincronización:**
- Cada semáforo conoce el estado del otro en tiempo real
- Si no recibe mensajes por > 2000 ms → Asume desconexión
- Los mensajes permiten coordinar transiciones de estado

### ⚠️ Seguridad y transiciones:

#### **Estado ALL_RED obligatorio:**
```cpp
void transitionToAllRed() {
  setLights(true, false, false);  // Rojo ON, Amarillo OFF, Verde OFF
  stateStartTime = millis();
  
  // Después de 1000ms → Decidir siguiente estado
}
```

**¿Por qué ALL_RED?**
- ✅ Evita que ambos semáforos estén en verde simultáneamente
- ✅ Da tiempo de seguridad para que vehículos crucen completamente
- ✅ Punto de sincronización entre ambos sistemas

#### **Regla de transición YELLOW → ALL_RED:**
```cpp
case YELLOW:
  if (elapsed >= YELLOW_DURATION) {  // 3000ms
    currentState = ALL_RED;
    // Nunca pasa directo de YELLOW a RED sin ALL_RED
  }
  break;
```

### 🔁 Ejemplo de ciclo completo con prioridad:

**Situación inicial:**
- Semáforo A en VERDE (8s transcurridos)
- Semáforo B en ROJO
- Ambos sin vehículos

**t = 8s:** Vehículo se acerca a B (4 cm)
- B detecta vehículo → `request = 1`
- A recibe solicitud → Evalúa extender verde o ceder turno
- A no tiene vehículos → Continúa ciclo normal (2s más)

**t = 10s:** A completa su verde
- A → YELLOW (3s)
- B → espera en RED con `request = 1`

**t = 13s:** A termina amarillo
- A → ALL_RED
- B → ALL_RED

**t = 14s:** Período de seguridad termina
- A → RED
- B → GREEN (tiene prioridad por vehículo detectado)

**t = 14-34s:** B en verde hasta 20s máximo
- Si vehículo sigue presente → Mantiene verde
- Si vehículo se aleja → Puede terminar antes

### 📊 Tiempos configurables:

| Parámetro | Valor | Modificable en |
|-----------|-------|----------------|
| Verde normal | 10s | `GREEN_NORMAL_DURATION` |
| Verde máximo | 20s | `MAX_GREEN_DURATION` |
| Amarillo | 3s | `YELLOW_DURATION` |
| All-Red | 1s | `ALL_RED_DURATION` |
| Detección threshold | 5 cm | `DETECTION_THRESHOLD_CM` |
| Persistencia | 500 ms | `DETECTION_PERSIST_MS` |
| Broadcast ESP-NOW | 200 ms | `loop()` delay |
| Timeout peer | 2000 ms | `peerTimeoutMs` |

---

## ⚙️ Instalación y uso

### 1️⃣ **Arduino IDE (ESP32):**

```bash
# Librerías requeridas:
- Adafruit SSD1306 (solo si habilitas OLED)
- Adafruit GFX Library (solo si habilitas OLED)
- esp_now.h (incluida en ESP32 core)

# Board:
ESP32S3 Dev Module

# Configuración:
Upload Speed: 921600
USB CDC On Boot: Enabled
```

### 2️⃣ **Python Monitor (opcional):**

```powershell
# Windows PowerShell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
python monitor_semaforos.py
```

```bash
# Linux/macOS
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python monitor_semaforos.py
```

---

## 🔧 Configuración de MACs (IMPORTANTE)

**Paso crítico:** Intercambiar direcciones MAC entre ESP32s

### Método rápido con `TEST_MAC.ino`:

1. Subir `TEST_MAC.ino` al **ESP A** → Copiar MAC (ej: `A4:CF:12:9E:6B:34`)
2. Subir `TEST_MAC.ino` al **ESP B** → Copiar MAC (ej: `B8:D6:1A:8F:2C:45`)
3. En `traffic_A.ino` línea 48: Poner **MAC de B**
   ```cpp
   uint8_t peerMAC[] = {0xB8, 0xD6, 0x1A, 0x8F, 0x2C, 0x45};  // MAC de B
   ```
4. En `traffic_B.ino` línea 48: Poner **MAC de A**
   ```cpp
   uint8_t peerMAC[] = {0xA4, 0xCF, 0x12, 0x9E, 0x6B, 0x34};  // MAC de A
   ```
5. Re-subir ambos códigos

👉 **Guía completa:** [`CONFIGURACION_MAC.md`](CONFIGURACION_MAC.md)

---

## ✅ Verificación de funcionamiento

### Serial Monitor esperado:

**ESP A:**
```
=== SEMAFORO A - Iniciando ===
OLED: Deshabilitado (no conectado)
MAC Address: A4:CF:12:9E:6B:34
Peer añadido correctamente
=== Sistema listo ===
TX: estado=0, request=0, dist=9999
RX de ESP 2: estado=3, request=0, dist=9999  ← ✅ RECIBE DE B
-> VERDE
```

**ESP B:**
```
=== SEMAFORO B - Iniciando ===
OLED: Deshabilitado (no conectado)
MAC Address: B8:D6:1A:8F:2C:45
Peer añadido correctamente
=== Sistema listo ===
TX: estado=3, request=0, dist=9999
RX de ESP 1: estado=0, request=0, dist=9999  ← ✅ RECIBE DE A
-> ROJO (turno de A)
```

### Señales de éxito:
- ✅ `Peer añadido correctamente`
- ✅ `RX de ESP 2` en ESP A
- ✅ `RX de ESP 1` en ESP B
- ✅ LEDs alternando automáticamente

---

## 🐛 Solución de problemas

| Problema | Documento | Solución rápida |
|----------|-----------|-----------------|
| Errores I2C NACK | [`SOLUCION_OLED.md`](SOLUCION_OLED.md) | OLED ya deshabilitado |
| Auto-recepción ESP-NOW | [`CONFIGURACION_MAC.md`](CONFIGURACION_MAC.md) | Configurar MACs reales |
| Logs confusos | [`ANALISIS_LOGS.md`](ANALISIS_LOGS.md) | Decodificación de mensajes |
| Setup desde cero | [`INICIO_RAPIDO.md`](INICIO_RAPIDO.md) | 5 pasos completos |

---

## 📚 Documentación completa

### Guías de inicio:
- **[`INICIO_RAPIDO.md`](INICIO_RAPIDO.md)** — Setup completo en 5 pasos (15-30 min)
- **[`CONFIGURACION_MAC.md`](CONFIGURACION_MAC.md)** — Configurar ESP-NOW paso a paso
- **[`TEST_MAC.ino`](TEST_MAC.ino)** — Sketch para obtener MAC rápidamente

### Solución de problemas:
- **[`SOLUCION_OLED.md`](SOLUCION_OLED.md)** — Errores I2C NACK, habilitar/deshabilitar OLED
- **[`ANALISIS_LOGS.md`](ANALISIS_LOGS.md)** — Interpretar logs Serial, decodificar estados
- **[`RESUMEN_CAMBIOS.md`](RESUMEN_CAMBIOS.md)** — Historial de modificaciones

### Código fuente:
- **[`traffic_A.ino`](traffic_A.ino)** — ESP32-S3 A (DEVICE_ID=1, pines 1,2,42,...)
- **[`traffic_B.ino`](traffic_B.ino)** — ESP32-S3 B (DEVICE_ID=2, pines 4,5,6,...)
- **[`monitor_semaforos.py`](monitor_semaforos.py)** — Monitor PyQt6 con GUI moderna

### Documentación técnica (carpeta `codbase/`):
- **[`DESIGN.md`](codbase/DESIGN.md)** — Arquitectura del sistema, máquina de estados
- **[`PROTOCOL.md`](codbase/PROTOCOL.md)** — Formato mensajes ESP-NOW (10 bytes)
- **[`PINOUT.md`](codbase/PINOUT.md)** — GPIO de ambos ESP32, diagrama de conexiones
- **[`TEST_PLAN.md`](codbase/TEST_PLAN.md)** — Plan de pruebas y validación
- **[`INSTALL.md`](codbase/INSTALL.md)** — Compilación e instalación de librerías
- **[`SETUP_A.md`](codbase/SETUP_A.md)** / **[`SETUP_B.md`](SETUP_B.md)** — Setup específico

---

## 📊 Especificaciones técnicas

### Tiempos del sistema:
- Verde normal: **10s**
- Verde extendido: **hasta 20s** (con vehículo)
- Amarillo: **3s**
- All-Red (seguridad): **1s**
- Broadcast ESP-NOW: **cada 200ms**

### Protocolo ESP-NOW:
```cpp
struct TrafficMsg {
  uint8_t sender_id;      // 1=ESP A, 2=ESP B
  uint8_t seq;            // Contador 0-255
  uint8_t state;          // 0=ALL_RED, 1=GREEN, 2=YELLOW, 3=RED, 4=WAIT
  uint8_t request;        // 0=no priority, 1=priority request
  uint16_t distance_cm;   // 0-9999 (9999 = no detection)
  uint32_t timestamp_ms;  // Millis()
};
```

### Detección de vehículos:
- Threshold: **< 100 cm**
- Persistencia: **1 segundo** (evitar falsos positivos)
- Extensión verde: **+3s por detección** (máx 20s total)

### Pin configuration:

| Función | ESP32-S3 A | ESP32-S3 B |
|---------|------------|------------|
| LED Rojo | GPIO 1 | GPIO 4 |
| LED Amarillo | GPIO 2 | GPIO 5 |
| LED Verde | GPIO 42 | GPIO 6 |
| HC-SR04 TRIG | GPIO 40 | GPIO 7 |
| HC-SR04 ECHO | GPIO 39 | GPIO 15 |
| Buzzer | GPIO 38 | GPIO 16 |
| OLED SDA | GPIO 21 | GPIO 17 |
| OLED SCL | GPIO 47 | GPIO 18 |

---

## 🎓 Créditos y licencia

**Proyecto académico**  
Universidad Militar Nueva Granada  
Curso: Bluetooth y NOW  
Fecha: Octubre 2025

**Autor:** Daniel Araque Studios  
**Licencia:** MIT (uso académico)

---

## 📞 Soporte

### ¿Problemas?
1. Revisa [`INICIO_RAPIDO.md`](INICIO_RAPIDO.md) primero
2. Busca el error en [`ANALISIS_LOGS.md`](ANALISIS_LOGS.md)
3. Consulta la documentación técnica en `codbase/`

### Errores comunes:
- ❌ **I2C NACK** → [`SOLUCION_OLED.md`](SOLUCION_OLED.md)
- ❌ **Auto-recepción** → [`CONFIGURACION_MAC.md`](CONFIGURACION_MAC.md)
- ❌ **LEDs no encienden** → Verificar GPIO en [`PINOUT.md`](codbase/PINOUT.md)
- ❌ **Sin comunicación** → Revisar MACs en línea 48 de ambos archivos

---

**¡Buena suerte con tu proyecto! 🚦✨**