# 🔧 Configuración de Direcciones MAC - ESP-NOW

## ⚠️ PROBLEMA ACTUAL

Los ESP32 están recibiendo sus propios mensajes porque ambos tienen la misma MAC configurada:
```cpp
uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // BROADCAST - INCORRECTO
```

## 📋 PASOS PARA CONFIGURAR CORRECTAMENTE

### **PASO 1: Obtener MAC de cada ESP32**

1. **Subir `traffic_A.ino` al primer ESP32**
   - Arduino IDE → Board: "ESP32S3 Dev Module"
   - Upload
   
2. **Abrir Serial Monitor** (115200 baud)
   - Buscar la línea: `MAC Address: XX:XX:XX:XX:XX:XX`
   - **COPIAR** esta dirección (ejemplo: `A4:CF:12:9E:6B:34`)
   
3. **Repetir con `traffic_B.ino` en el segundo ESP32**
   - Upload
   - **COPIAR** su MAC (ejemplo: `B8:D6:1A:8F:2C:45`)

---

### **PASO 2: Intercambiar las MACs**

#### **En `traffic_A.ino` (línea 48):**
```cpp
// Reemplazar con la MAC del ESP B
uint8_t peerMAC[] = {0xB8, 0xD6, 0x1A, 0x8F, 0x2C, 0x45};  // ← MAC del ESP B
```

#### **En `traffic_B.ino` (línea 48):**
```cpp
// Reemplazar con la MAC del ESP A
uint8_t peerMAC[] = {0xA4, 0xCF, 0x12, 0x9E, 0x6B, 0x34};  // ← MAC del ESP A
```

**Convertir formato:**
- De: `A4:CF:12:9E:6B:34`
- A: `{0xA4, 0xCF, 0x12, 0x9E, 0x6B, 0x34}`

---

### **PASO 3: Re-subir código**

1. Volver a compilar y subir `traffic_A.ino` al ESP A
2. Volver a compilar y subir `traffic_B.ino` al ESP B
3. Abrir ambos Serial Monitors y verificar:
   - ✅ `[A] RX de ESP 2: estado=...` (ESP A recibe mensajes de ESP B)
   - ✅ `[B] RX de ESP 1: estado=...` (ESP B recibe mensajes de ESP A)

---

## ✅ VERIFICACIÓN EXITOSA

Deberías ver en el monitor Python:
```
[A] MAC Address: A4:CF:12:9E:6B:34
[A] Peer añadido correctamente
[A] TX: estado=0, request=0, dist=9999
[A] RX de ESP 2: estado=3, request=0, dist=9999  ← ✅ RECIBE DE B

[B] MAC Address: B8:D6:1A:8F:2C:45
[B] Peer añadido correctamente
[B] TX: estado=3, request=0, dist=9999
[B] RX de ESP 1: estado=0, request=0, dist=9999  ← ✅ RECIBE DE A
```

---

## 🔍 SOLUCIÓN DE PROBLEMAS

### Error: "Error añadiendo peer"
- Verificar que las MACs estén en formato hexadecimal: `0xXX`
- Revisar que no haya errores de tipeo

### Siguen recibiéndose a sí mismos
- Confirmar que las MACs intercambiadas sean correctas
- Ambos ESP32 deben tener MACs **DIFERENTES** en `peerMAC[]`

### No hay comunicación ESP-NOW
- Verificar que ambos ESP32 estén en el mismo canal WiFi (canal 0 = auto)
- Confirmar que `Peer añadido correctamente` aparezca en ambos

---

## 📝 EJEMPLO COMPLETO

**ESP A detectó:**
```
MAC Address: A4:CF:12:9E:6B:34
```

**ESP B detectó:**
```
MAC Address: B8:D6:1A:8F:2C:45
```

**Configuración final:**

`traffic_A.ino`:
```cpp
uint8_t peerMAC[] = {0xB8, 0xD6, 0x1A, 0x8F, 0x2C, 0x45};  // MAC de B
```

`traffic_B.ino`:
```cpp
uint8_t peerMAC[] = {0xA4, 0xCF, 0x12, 0x9E, 0x6B, 0x34};  // MAC de A
```
