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

### Con detección de vehículo:
- **Verde extendido:** Hasta 20s si hay vehículo (<100cm)
- **Prioridad:** Carril con vehículo más cercano
- **Fairness:** Alternancia automática cada N ciclos

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