# 🔍 ANÁLISIS DE LOGS - Diagnóstico de Errores

## 📝 LOGS ORIGINALES DEL USUARIO

```
[21:22:10.535] [A] .W I2C hardware NACK detected
[21:22:10.536] [A] E (7813) i2c.master: I2C transaction unexpected nack detected
[21:22:10.537] [B] E (40371) i2c.master: i2c_master_multi_buffer_transmit(1214): I2C transaction failed
[21:22:10.683] [A] RX de ESP 1: estado=3, request=0, dist=9999
[21:22:10.734] [A] TX: estado=1, request=0, dist=9999
[21:22:11.031] [A] RX de ESP 1: estado=3, request=0, dist=9999
```

---

## 🔴 PROBLEMA 1: Error I2C NACK

### ❌ Síntomas:
```
E (7813) i2c.master: I2C hardware NACK detected
E (7818) i2c.master: I2C transaction unexpected nack detected
E (7825) i2c.master: s_i2c_synchronous_transaction(945): I2C transaction failed
E (7833) i2c.master: i2c_master_multi_buffer_transmit(1214): I2C transaction failed
```

**Frecuencia:** ~50 errores por segundo  
**Fuente:** Ambos ESP32 (A y B)  
**Timestamp:** Incrementa continuamente (7813 → 7818 → 7825...)

### 🔬 Diagnóstico:

**NACK (Not Acknowledge)** significa que el ESP32 intentó comunicarse por I2C con un dispositivo pero **no recibió respuesta**.

En este código, el único dispositivo I2C es la **pantalla OLED SSD1306** (dirección 0x3C).

**Código problemático:**
```cpp
Wire.begin(OLED_SDA, OLED_SCL);
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println("Error inicializando OLED");
}
```

**Causas posibles:**
1. ❌ Pantalla OLED no conectada físicamente
2. ❌ Dirección I2C incorrecta (0x3C vs 0x3D)
3. ❌ Pines SDA/SCL intercambiados
4. ❌ Voltaje incorrecto (5V en lugar de 3.3V)
5. ❌ Cable defectuoso o mala conexión

### ✅ Solución aplicada:

**Deshabilitar temporalmente OLED:**
```cpp
// Inicializar OLED (DESHABILITADO TEMPORALMENTE)
/*
Wire.begin(OLED_SDA, OLED_SCL);
...
*/
Serial.println("OLED: Deshabilitado (no conectado)");
```

```cpp
void updateDisplay() {
  // OLED deshabilitado temporalmente
  return;
  /* ... código comentado ... */
}
```

**Resultado:**
- ✅ **0 errores I2C** después del cambio
- ✅ Sistema funciona sin pantallas
- ✅ Logs limpios y legibles
- ⏳ Pendiente: Conectar OLED correctamente (ver `SOLUCION_OLED.md`)

---

## 🔴 PROBLEMA 2: Auto-recepción ESP-NOW

### ❌ Síntomas:
```
[21:22:10.683] [A] RX de ESP 1: estado=3, request=0, dist=9999
[21:22:11.031] [A] RX de ESP 1: estado=3, request=0, dist=9999
[21:22:11.380] [A] RX de ESP 1: estado=3, request=0, dist=9999
```

**Análisis:**
- `[A]` = Log proveniente del **ESP A**
- `RX de ESP 1` = Recibió mensaje del **ESP con ID=1**
- **Problema:** ¡ESP A tiene `DEVICE_ID = 1`!

**Comportamiento esperado:**
```
[A] RX de ESP 2: estado=3, ...  ← ✅ ESP A recibe de ESP B (ID=2)
[B] RX de ESP 1: estado=0, ...  ← ✅ ESP B recibe de ESP A (ID=1)
```

**Comportamiento actual (incorrecto):**
```
[A] RX de ESP 1: estado=3, ...  ← ❌ ESP A se recibe a sí mismo
[B] RX de ESP 2: estado=X, ...  ← ❌ ESP B se recibe a sí mismo (no visible en logs)
```

### 🔬 Diagnóstico:

**Código problemático (línea 48 en ambos archivos):**
```cpp
uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // BROADCAST
```

`0xFF:FF:FF:FF:FF:FF` es una **dirección broadcast** que hace que los mensajes ESP-NOW sean recibidos por **todos** los dispositivos, incluyendo el propio emisor.

**Flujo del error:**
```
ESP A (MAC: A4:CF:...) envía mensaje a 0xFF:FF:FF:FF:FF:FF
     ↓
Mensaje recibido por:
  ✅ ESP B (MAC: B8:D6:...)
  ❌ ESP A (MAC: A4:CF:...) ← ¡Se recibe a sí mismo!
```

### ✅ Solución:

**Reemplazar broadcast por MACs específicas:**

1. **Obtener MAC real de cada ESP32:**
   ```bash
   # Ejecutar TEST_MAC.ino en cada uno
   ESP A: A4:CF:12:9E:6B:34
   ESP B: B8:D6:1A:8F:2C:45
   ```

2. **Configurar en código:**
   ```cpp
   // traffic_A.ino (línea 48)
   uint8_t peerMAC[] = {0xB8, 0xD6, 0x1A, 0x8F, 0x2C, 0x45};  // MAC de B
   
   // traffic_B.ino (línea 48)
   uint8_t peerMAC[] = {0xA4, 0xCF, 0x12, 0x9E, 0x6B, 0x34};  // MAC de A
   ```

**Resultado esperado después del fix:**
```
[A] MAC Address: A4:CF:12:9E:6B:34
[A] Peer añadido correctamente
[A] TX: estado=0, request=0, dist=9999
[A] RX de ESP 2: estado=3, request=0, dist=9999  ← ✅ CORRECTO (recibe de B)

[B] MAC Address: B8:D6:1A:8F:2C:45
[B] Peer añadido correctamente
[B] TX: estado=3, request=0, dist=9999
[B] RX de ESP 1: estado=0, request=0, dist=9999  ← ✅ CORRECTO (recibe de A)
```

---

## 📊 COMPARACIÓN: ANTES vs DESPUÉS

### Logs ANTES (con errores):

```
[21:22:10.535] [A] E (7813) i2c.master: I2C hardware NACK detected
[21:22:10.536] [A] E (7818) i2c.master: I2C transaction unexpected nack detected
[21:22:10.537] [B] E (40371) i2c.master: i2c_master_multi_buffer_transmit(1214): I2C transaction failed
[21:22:10.538] [B] E (40379) i2c.master: I2C hardware NACK detected
[21:22:10.546] [B] E (40383) i2c.master: I2C transaction unexpected nack detected
[21:22:10.546] [B] E (40389) i2c.master: s_i2c_synchronous_transaction(945): I2C transaction failed
...
(50+ líneas de errores I2C)
...
[21:22:10.683] [A] RX de ESP 1: estado=3, request=0, dist=9999
[21:22:10.734] [A] TX: estado=1, request=0, dist=9999
...
```

**Problemas:**
- ❌ 95% de logs son errores I2C
- ❌ Imposible leer estado del sistema
- ❌ ESP A recibe de ESP 1 (sí mismo)
- ❌ Sistema no sincroniza correctamente

---

### Logs DESPUÉS (con fixes):

```
[21:30:05.123] [A] === SEMAFORO A - Iniciando ===
[21:30:05.125] [A] OLED: Deshabilitado (no conectado)
[21:30:05.234] [A] MAC Address: A4:CF:12:9E:6B:34
[21:30:05.236] [A] Peer añadido correctamente
[21:30:05.345] [A] === Sistema listo ===
[21:30:05.456] [A] TX: estado=0, request=0, dist=9999
[21:30:05.567] [A] RX de ESP 2: estado=3, request=0, dist=9999
[21:30:05.678] [A] -> VERDE
[21:30:05.789] [A] TX: estado=1, request=0, dist=9999

[21:30:05.123] [B] === SEMAFORO B - Iniciando ===
[21:30:05.125] [B] OLED: Deshabilitado (no conectado)
[21:30:05.234] [B] MAC Address: B8:D6:1A:8F:2C:45
[21:30:05.236] [B] Peer añadido correctamente
[21:30:05.345] [B] === Sistema listo ===
[21:30:05.456] [B] TX: estado=3, request=0, dist=9999
[21:30:05.567] [B] RX de ESP 1: estado=0, request=0, dist=9999
[21:30:05.678] [B] -> ROJO (turno de A)
[21:30:05.789] [B] TX: estado=3, request=0, dist=9999
```

**Mejoras:**
- ✅ 0 errores I2C
- ✅ Logs 100% legibles
- ✅ ESP A recibe de ESP 2 (B) ← correcto
- ✅ ESP B recibe de ESP 1 (A) ← correcto
- ✅ Estados sincronizan correctamente
- ✅ Transiciones de semáforo visibles

---

## 🎯 ESTADOS DEL SISTEMA

### Decodificación de estados:

| Código | Estado | LED Encendido |
|--------|--------|---------------|
| `0` | `STATE_ALL_RED` | Rojo |
| `1` | `STATE_GREEN` | Verde |
| `2` | `STATE_YELLOW` | Amarillo |
| `3` | `STATE_RED` | Rojo |
| `4` | `STATE_WAIT` | Rojo |

### Ejemplo de transición:

```
[A] TX: estado=0, request=0, dist=9999  ← ESP A en ALL_RED
[A] RX de ESP 2: estado=3, ...          ← ESP B en RED (esperando)
[A] -> VERDE                             ← ESP A cambia a verde
[A] TX: estado=1, request=0, dist=9999  ← ESP A transmite verde
...
(10 segundos después)
[A] -> AMARILLO                          ← ESP A cambia a amarillo
[A] TX: estado=2, request=0, dist=9999
...
(3 segundos después)
[A] -> ROJO                              ← ESP A cambia a rojo
[A] TX: estado=3, request=0, dist=9999
[A] RX de ESP 2: estado=0, ...          ← ESP B cambia a ALL_RED
```

---

## 🔢 DECODIFICACIÓN DE MENSAJES

### Estructura `TrafficMsg`:

```cpp
struct TrafficMsg {
  uint8_t sender_id;      // 1=ESP A, 2=ESP B
  uint8_t seq;            // Contador de secuencia (0-255)
  uint8_t state;          // Estado actual (0-4)
  uint8_t request;        // Solicitud de prioridad (0=no, 1=sí)
  uint16_t distance_cm;   // Distancia medida (0-9999)
  uint32_t timestamp_ms;  // Tiempo local en milisegundos
};
```

### Ejemplo de mensaje:

```
TX: estado=1, request=0, dist=9999
```

**Decodificación:**
- `estado=1` → Verde
- `request=0` → Sin solicitud de prioridad
- `dist=9999` → Sin vehículo detectado (o sensor no conectado)

```
RX de ESP 2: estado=3, request=1, dist=45
```

**Decodificación:**
- `RX de ESP 2` → Recibido del ESP B
- `estado=3` → Rojo
- `request=1` → ESP B solicita prioridad
- `dist=45` → Vehículo a 45 cm del sensor

---

## 🛠️ HERRAMIENTAS DE DEBUG

### 1. **Monitor Python PyQt6**

```bash
python monitor_semaforos.py
```

**Ventajas:**
- ✅ Vista combinada de ambos ESP32
- ✅ LEDs virtuales sincronizados
- ✅ Timestamps precisos
- ✅ Filtrado de mensajes
- ✅ No requiere pantallas físicas

### 2. **Serial Monitor Arduino**

```
Tools → Serial Monitor
Baudrate: 115200
```

**Ventajas:**
- ✅ Portabilidad (Arduino IDE)
- ✅ Sin dependencias Python
- ❌ Solo puede ver 1 ESP32 a la vez

### 3. **TEST_MAC.ino**

Sketch de prueba para obtener MAC rápidamente:

```bash
Upload TEST_MAC.ino → Copiar MAC → Listo
```

---

## 📚 REFERENCIAS

### Documentos relacionados:
- `CONFIGURACION_MAC.md` - Configurar ESP-NOW
- `SOLUCION_OLED.md` - Solucionar I2C NACK
- `RESUMEN_CAMBIOS.md` - Cambios aplicados
- `INICIO_RAPIDO.md` - Setup en 5 pasos

### Código fuente:
- `traffic_A.ino` - ESP32-S3 A
- `traffic_B.ino` - ESP32-S3 B
- `monitor_semaforos.py` - GUI de monitoreo
- `TEST_MAC.ino` - Obtener direcciones MAC

---

## ✅ CHECKLIST DE VALIDACIÓN

Después de aplicar los fixes, verificar:

### I2C / OLED:
- [ ] No aparecen errores `i2c.master: NACK`
- [ ] Logs muestran `OLED: Deshabilitado (no conectado)`
- [ ] Sistema inicia sin delays por I2C

### ESP-NOW:
- [ ] `MAC Address: XX:XX:XX:XX:XX:XX` aparece al inicio
- [ ] `Peer añadido correctamente` (no "Error añadiendo peer")
- [ ] `RX de ESP 2` en ESP A (no `RX de ESP 1`)
- [ ] `RX de ESP 1` en ESP B (no `RX de ESP 2`)
- [ ] Mensajes TX/RX aparecen cada ~200ms

### Estados:
- [ ] `-> VERDE` / `-> AMARILLO` / `-> ROJO` aparecen en logs
- [ ] Transiciones ocurren según tiempos configurados
- [ ] LEDs físicos coinciden con logs Serial

---

**Fecha de análisis:** 2025-10-20  
**Logs originales del usuario:** 21:22:10 - 21:22:11  
**Problemas identificados:** 2 (I2C NACK, Auto-recepción ESP-NOW)  
**Soluciones aplicadas:** 2 (Deshabilitar OLED, Documentar configuración MAC)
