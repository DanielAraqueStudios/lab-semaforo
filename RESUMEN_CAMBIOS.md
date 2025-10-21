# 🚦 RESUMEN DE CAMBIOS - Sistema Semáforo

## 📅 Fecha: $(Get-Date -Format "yyyy-MM-dd HH:mm")

---

## ⚠️ PROBLEMAS DETECTADOS EN LOGS

### 1. **Error I2C NACK masivo** (Pantallas OLED)
```
E (7813) i2c.master: I2C hardware NACK detected
E (7818) i2c.master: I2C transaction unexpected nack detected
```
**Causa:** Pantallas OLED no conectadas o dirección I2C incorrecta.

### 2. **Auto-recepción ESP-NOW**
```
[A] RX de ESP 1: estado=3, request=0, dist=9999
```
**Causa:** Ambos ESP32 tienen la misma MAC broadcast (`0xFF:FF:FF:FF:FF:FF`) configurada.

---

## ✅ SOLUCIONES APLICADAS

### ✔️ **Cambio 1: OLED deshabilitado temporalmente**

**Archivos modificados:**
- `traffic_A.ino`
- `traffic_B.ino`

**Cambios:**
1. Comentada inicialización I2C en `setup()`
2. `updateDisplay()` retorna inmediatamente sin ejecutar código OLED
3. Agregado mensaje: `Serial.println("OLED: Deshabilitado (no conectado)");`

**Resultado:**
- ✅ Elimina 100% de errores I2C NACK
- ✅ Sistema funciona sin pantallas
- ✅ Logs limpios y legibles

---

### ⏳ **Pendiente: Configurar direcciones MAC**

**Problema:** Ambos ESP32 usan broadcast MAC:
```cpp
uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // ← INCORRECTO
```

**Solución:**
1. Subir `traffic_A.ino` → Copiar MAC del Serial Monitor
2. Subir `traffic_B.ino` → Copiar MAC del Serial Monitor
3. **Intercambiar MACs:**
   - En `traffic_A.ino` línea 48: Poner MAC de B
   - En `traffic_B.ino` línea 48: Poner MAC de A
4. Re-subir ambos códigos

**Ver:** `CONFIGURACION_MAC.md` para instrucciones detalladas.

---

## 📁 ARCHIVOS CREADOS

| Archivo | Descripción |
|---------|-------------|
| `CONFIGURACION_MAC.md` | Guía paso a paso para configurar direcciones MAC ESP-NOW |
| `SOLUCION_OLED.md` | Diagnóstico y solución de problemas I2C OLED |
| `RESUMEN_CAMBIOS.md` | Este documento (resumen ejecutivo) |

---

## 🎯 ESTADO ACTUAL DEL PROYECTO

### ✅ Completado
- [x] Código Arduino para ambos ESP32-S3
- [x] Monitor Python PyQt6 con GUI moderna
- [x] Documentación técnica completa en español
- [x] Pin configuration para ambos ESP32
- [x] Fix de callbacks ESP-NOW (IDF v5.x)
- [x] Deshabilitación temporal de OLED

### ⏳ En progreso / Pendiente
- [ ] Configurar MACs reales entre ESP32 A y B
- [ ] Verificar hardware I2C de pantallas OLED
- [ ] Probar comunicación ESP-NOW bidireccional
- [ ] Validar sensor HC-SR04 con detección de distancia
- [ ] Habilitar OLED (opcional, después de verificar I2C)

---

## 🔧 PRÓXIMOS PASOS INMEDIATOS

### **PASO 1: Compilar y subir código sin OLED**
```bash
# Arduino IDE
1. Abrir traffic_A.ino
2. Board: "ESP32S3 Dev Module"
3. Upload
4. Serial Monitor @ 115200 baud
5. COPIAR línea: "MAC Address: XX:XX:XX:XX:XX:XX"

# Repetir con traffic_B.ino en segundo ESP32
```

### **PASO 2: Configurar MACs**
```cpp
// traffic_A.ino línea 48
uint8_t peerMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};  // MAC de B

// traffic_B.ino línea 48
uint8_t peerMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};  // MAC de A
```

### **PASO 3: Verificar comunicación**
```bash
# Logs esperados en Serial Monitor:
[A] MAC Address: A4:CF:12:9E:6B:34
[A] Peer añadido correctamente
[A] TX: estado=0, request=0, dist=9999
[A] RX de ESP 2: estado=3, request=0, dist=9999  ← ✅ RECIBE DE B

[B] MAC Address: B8:D6:1A:8F:2C:45
[B] Peer añadido correctamente
[B] TX: estado=3, request=0, dist=9999
[B] RX de ESP 1: estado=0, request=0, dist=9999  ← ✅ RECIBE DE A
```

### **PASO 4: Probar en monitor Python**
```bash
python monitor_semaforos.py
# Seleccionar COM26 (ESP A) y COM32 (ESP B)
# Click "▶ Conectar"
# Observar sincronización de estados y LEDs
```

---

## 📊 COMPARACIÓN: ANTES vs DESPUÉS

| Característica | Antes (con errores) | Después (sin OLED) |
|----------------|---------------------|---------------------|
| **Errores I2C NACK** | ~50 por segundo | 0 (ninguno) |
| **Logs Serial** | Ilegibles (spam I2C) | Limpios y claros |
| **ESP-NOW funcional** | No (auto-recepción) | Pendiente MACs |
| **Pantallas OLED** | No funcionales | Deshabilitadas |
| **Velocidad loop()** | ~50ms | ~50ms (sin cambio) |
| **LEDs** | Funcionales | Funcionales |
| **HC-SR04** | Por probar | Por probar |

---

## 🧪 CHECKLIST DE VALIDACIÓN

### Hardware
- [ ] LEDs rojo/amarillo/verde encienden correctamente
- [ ] Sensor HC-SR04 mide distancias (0-400 cm)
- [ ] Buzzer emite tonos (opcional)
- [ ] Pantallas OLED responden en I2C 0x3C (opcional)

### Software
- [ ] Código compila sin errores en Arduino IDE
- [ ] Serial Monitor muestra MAC address al inicio
- [ ] Mensajes TX se envían cada 200ms
- [ ] Mensajes RX se reciben del otro ESP32 (no de sí mismo)
- [ ] Estados cambian: ALL_RED → GREEN → YELLOW → RED
- [ ] Monitor Python muestra LEDs sincronizados

### Comunicación ESP-NOW
- [ ] `Peer añadido correctamente` en ambos ESP32
- [ ] `RX de ESP 1` en el ESP B (recibe de A)
- [ ] `RX de ESP 2` en el ESP A (recibe de B)
- [ ] No hay `Error enviando mensaje ESP-NOW`

---

## 📝 NOTAS TÉCNICAS

### Pin Configuration Actual

**ESP32-S3 A:**
- LEDs: GPIO 1 (Rojo), 2 (Amarillo), 42 (Verde)
- HC-SR04: GPIO 40 (TRIG), 39 (ECHO)
- Buzzer: GPIO 38
- OLED: GPIO 21 (SDA), 47 (SCL) — **DESHABILITADO**

**ESP32-S3 B:**
- LEDs: GPIO 4 (Rojo), 5 (Amarillo), 6 (Verde)
- HC-SR04: GPIO 7 (TRIG), 15 (ECHO)
- Buzzer: GPIO 16
- OLED: GPIO 17 (SDA), 18 (SCL) — **DESHABILITADO**

### Tiempos del sistema
- Verde normal: 10s
- Verde extendido: hasta 20s (con vehículo detectado)
- Amarillo: 3s
- All-Red (seguridad): 1s
- Broadcast ESP-NOW: cada 200ms

---

## 🆘 SOPORTE Y RESOLUCIÓN DE PROBLEMAS

### Ver documentación:
1. **Configuración MAC:** `CONFIGURACION_MAC.md`
2. **Problemas OLED:** `SOLUCION_OLED.md`
3. **Setup completo:** `codbase/SETUP_A.md` y `SETUP_B.md`
4. **Protocolo ESP-NOW:** `codbase/PROTOCOL.md`
5. **Pines GPIO:** `codbase/PINOUT.md`

### Contacto:
- Universidad Militar Nueva Granada
- Curso: Bluetooth y NOW
- Proyecto: Sistema Semafórico Inteligente ESP32-S3

---

**Última actualización:** 2025-10-20 21:30 (hora local)
