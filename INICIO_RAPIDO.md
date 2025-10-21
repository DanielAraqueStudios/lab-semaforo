# 🚀 INICIO RÁPIDO - Sistema Semáforo ESP32-S3

## ⚡ 5 PASOS PARA PONER EN FUNCIONAMIENTO

---

## 📋 PASO 1: Obtener direcciones MAC

### Opción A: Usar `TEST_MAC.ino` (Recomendado)

1. Abrir Arduino IDE
2. File → Open → `TEST_MAC.ino`
3. Tools → Board → "ESP32S3 Dev Module"
4. Tools → Port → Seleccionar puerto COM del primer ESP32
5. Click Upload (→)
6. Tools → Serial Monitor (Ctrl+Shift+M)
7. **COPIAR** la línea que empieza con `uint8_t peerMAC[] = {...}`

**Ejemplo de salida:**
```
===========================================
    TEST DE DIRECCIÓN MAC - ESP32-S3
===========================================

✅ MAC Address: A4:CF:12:9E:6B:34

📋 Formato para código Arduino:
   uint8_t peerMAC[] = {0xA4, 0xCF, 0x12, 0x9E, 0x6B, 0x34};

===========================================
```

8. **Guardar esta MAC** (ejemplo: "MAC_A.txt")
9. **Repetir** con el segundo ESP32 y guardar como "MAC_B.txt"

---

## 🔄 PASO 2: Configurar MACs en el código

### ESP32 A:

1. Abrir `traffic_A.ino`
2. Ir a **línea 48**
3. Reemplazar:
   ```cpp
   uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // CAMBIAR ESTO
   ```
   Con la **MAC del ESP B** (la segunda que obtuviste):
   ```cpp
   uint8_t peerMAC[] = {0xB8, 0xD6, 0x1A, 0x8F, 0x2C, 0x45};  // MAC de B
   ```

### ESP32 B:

1. Abrir `traffic_B.ino`
2. Ir a **línea 48**
3. Reemplazar con la **MAC del ESP A** (la primera):
   ```cpp
   uint8_t peerMAC[] = {0xA4, 0xCF, 0x12, 0x9E, 0x6B, 0x34};  // MAC de A
   ```

4. **Guardar ambos archivos** (Ctrl+S)

---

## 🔌 PASO 3: Conexión de hardware

### Mínimo necesario (sin OLED, sin sensor):

**ESP32-S3 A:**
```
LED Rojo    → GPIO 1  + Resistencia 220Ω → GND
LED Amarillo→ GPIO 2  + Resistencia 220Ω → GND
LED Verde   → GPIO 42 + Resistencia 220Ω → GND
```

**ESP32-S3 B:**
```
LED Rojo    → GPIO 4  + Resistencia 220Ω → GND
LED Amarillo→ GPIO 5  + Resistencia 220Ω → GND
LED Verde   → GPIO 6  + Resistencia 220Ω → GND
```

### Opcional - Sensor HC-SR04 (detección de vehículos):

**ESP32-S3 A:**
```
HC-SR04:
  VCC  → 5V
  TRIG → GPIO 40
  ECHO → GPIO 39 (⚠️ usar divisor de voltaje 5V → 3.3V)
  GND  → GND
```

**ESP32-S3 B:**
```
HC-SR04:
  VCC  → 5V
  TRIG → GPIO 7
  ECHO → GPIO 15 (⚠️ usar divisor de voltaje 5V → 3.3V)
  GND  → GND
```

**Divisor de voltaje ECHO:**
```
      5V ECHO ----[1kΩ]---- GPIO (3.3V)
                     |
                  [2kΩ]
                     |
                    GND
```

---

## 📤 PASO 4: Subir código a los ESP32

### ESP32 A:

1. Arduino IDE → Open → `traffic_A.ino`
2. Tools → Board → "ESP32S3 Dev Module"
3. Tools → Port → Seleccionar puerto COM del ESP A
4. Sketch → Upload (→)
5. Esperar "Done uploading"

### ESP32 B:

1. Arduino IDE → Open → `traffic_B.ino`
2. Tools → Port → Seleccionar puerto COM del ESP B
3. Sketch → Upload (→)
4. Esperar "Done uploading"

---

## ✅ PASO 5: Verificar comunicación

### Abrir Serial Monitors:

**Monitor ESP A:**
```
=== SEMAFORO A - Iniciando ===
OLED: Deshabilitado (no conectado)
MAC Address: A4:CF:12:9E:6B:34
Peer añadido correctamente
=== Sistema listo ===
TX: estado=0, request=0, dist=9999
RX de ESP 2: estado=3, request=0, dist=9999  ← ✅ CORRECTO
-> VERDE
TX: estado=1, request=0, dist=9999
```

**Monitor ESP B:**
```
=== SEMAFORO B - Iniciando ===
OLED: Deshabilitado (no conectado)
MAC Address: B8:D6:1A:8F:2C:45
Peer añadido correctamente
=== Sistema listo ===
TX: estado=3, request=0, dist=9999
RX de ESP 1: estado=0, request=0, dist=9999  ← ✅ CORRECTO
-> ROJO (turno de A)
```

**✅ Señales de éxito:**
- `Peer añadido correctamente`
- `RX de ESP 1` (en B) y `RX de ESP 2` (en A)
- LEDs cambiando de estado automáticamente

**❌ Señales de error:**
- `Error añadiendo peer` → Verificar MACs
- `RX de ESP 1` en el **mismo** ESP A → MACs mal configuradas
- `Error enviando mensaje ESP-NOW` → Reiniciar ESP32

---

## 🐍 PASO 6 (Opcional): Monitor Python

### Ejecutar GUI de monitoreo:

```powershell
# Activar entorno virtual (si existe)
.\.venv\Scripts\Activate.ps1

# Instalar dependencias
pip install -r requirements.txt

# Ejecutar monitor
python monitor_semaforos.py
```

### Uso del monitor:

1. **Puerto A:** Seleccionar COM del ESP A (ej: COM26)
2. **Puerto B:** Seleccionar COM del ESP B (ej: COM32)
3. Click "**▶ Conectar**"
4. Observar:
   - LEDs virtuales sincronizados con hardware
   - Logs en tiempo real
   - Distancia del sensor (si está conectado)
   - Estado de sincronización ESP-NOW

---

## 🎯 COMPORTAMIENTO ESPERADO

### Ciclo normal (sin vehículos):

1. **Inicio:** Ambos en ALL_RED (1 segundo)
2. **ESP A → Verde** (10 segundos), ESP B → Rojo
3. **ESP A → Amarillo** (3 segundos), ESP B → Rojo
4. **ESP A → Rojo**, ESP B → Rojo (1 segundo)
5. **ESP B → Verde** (10 segundos), ESP A → Rojo
6. **ESP B → Amarillo** (3 segundos), ESP A → Rojo
7. **Vuelta al paso 1** (ciclo se repite)

### Con sensor HC-SR04 detectando vehículo:

- Verde se extiende hasta **20 segundos** (máximo)
- Sistema prioriza el carril con vehículo más cercano
- Si ambos detectan, gana el de menor distancia

---

## 🔍 SOLUCIÓN DE PROBLEMAS COMUNES

### ❌ "Error añadiendo peer"

**Causa:** MAC incorrecta o mal formateada

**Solución:**
1. Verificar que las MACs tengan formato `0xXX, 0xXX, ...`
2. Confirmar que cada ESP tenga la MAC del **otro**
3. Re-subir código

---

### ❌ "RX de ESP 1" aparece en ESP A (auto-recepción)

**Causa:** Ambos ESP tienen la misma MAC (broadcast)

**Solución:**
1. Ejecutar `TEST_MAC.ino` en **ambos** ESP32
2. **Intercambiar** las MACs correctamente
3. Re-subir código

---

### ❌ LEDs no encienden

**Causa:** GPIO mal conectado o resistencia incorrecta

**Solución:**
1. Verificar continuidad con multímetro
2. Probar LED directamente (3.3V → LED → 220Ω → GND)
3. Revisar número de GPIO en código vs hardware

---

### ❌ Sensor HC-SR04 siempre marca 9999 cm

**Causa:** Cables incorrectos o voltaje ECHO incorrecto

**Solución:**
1. Verificar TRIG/ECHO no estén intercambiados
2. Asegurar divisor de voltaje en ECHO (5V → 3.3V)
3. Probar con sketch de prueba simple

---

### ❌ Monitor Python: "Error al conectar"

**Causa:** Puerto COM incorrecto o ESP32 no conectado

**Solución:**
1. Verificar ESP32 conectado por USB
2. Click "🔄 Refresh Ports"
3. Cerrar otros Serial Monitors (Arduino IDE)
4. Reintentar conexión

---

## 📚 DOCUMENTACIÓN COMPLETA

### Guías técnicas:
- `CONFIGURACION_MAC.md` - Configuración ESP-NOW
- `SOLUCION_OLED.md` - Habilitar pantallas I2C
- `RESUMEN_CAMBIOS.md` - Historial de modificaciones

### Carpeta `codbase/`:
- `DESIGN.md` - Arquitectura del sistema
- `PROTOCOL.md` - Formato de mensajes ESP-NOW
- `PINOUT.md` - Configuración de pines GPIO
- `SETUP_A.md` / `SETUP_B.md` - Setup específico por ESP32
- `TEST_PLAN.md` - Plan de pruebas

---

## ✨ RESUMEN DE 5 PASOS

```
1. 📡 Obtener MACs con TEST_MAC.ino
2. ✏️  Editar traffic_A.ino (línea 48) y traffic_B.ino (línea 48)
3. 🔌 Conectar LEDs (mínimo 3 por ESP32)
4. 📤 Subir código a ambos ESP32
5. ✅ Verificar "RX de ESP 2" en A y "RX de ESP 1" en B
```

**¡Listo!** El sistema debería estar funcionando con LEDs alternando automáticamente.

---

**Última actualización:** 2025-10-20  
**Contacto:** Universidad Militar Nueva Granada - Bluetooth y NOW
