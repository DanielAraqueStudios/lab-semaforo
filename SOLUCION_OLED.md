# 🖥️ Solución de Problemas - Pantallas OLED I2C

## ❌ ERROR DETECTADO

Los logs muestran errores masivos de I2C NACK:
```
E (7813) i2c.master: I2C hardware NACK detected
E (7818) i2c.master: I2C transaction unexpected nack detected
E (7825) i2c.master: s_i2c_synchronous_transaction(945): I2C transaction failed
```

**Causa:** Las pantallas OLED SSD1306 no están respondiendo en la dirección I2C `0x3C`.

---

## ✅ SOLUCIÓN APLICADA (Temporal)

Se **deshabilitó temporalmente** el código OLED en ambos archivos (`traffic_A.ino` y `traffic_B.ino`) para permitir que el sistema funcione sin pantallas.

### Cambios realizados:

#### 1. **Función `setup()` - Inicialización OLED comentada**
```cpp
// Inicializar OLED (DESHABILITADO TEMPORALMENTE)
/*
Wire.begin(OLED_SDA, OLED_SCL);
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println("Error inicializando OLED");
} else {
  Serial.println("OLED OK");
  ...
}
*/
Serial.println("OLED: Deshabilitado (no conectado)");
```

#### 2. **Función `updateDisplay()` - Return inmediato**
```cpp
void updateDisplay() {
  // OLED deshabilitado temporalmente
  return;
  
  /* ... código OLED comentado ... */
}
```

---

## 🔌 HABILITAR OLED NUEVAMENTE (Opcional)

Si quieres usar las pantallas OLED, sigue estos pasos:

### **PASO 1: Verificar conexión física**

| Pin ESP32-S3 A | Pin OLED | Pin ESP32-S3 B | Pin OLED |
|----------------|----------|----------------|----------|
| GPIO 21        | SDA      | GPIO 17        | SDA      |
| GPIO 47        | SCL      | GPIO 18        | SCL      |
| GND            | GND      | GND            | GND      |
| 3.3V           | VCC      | 3.3V           | VCC      |

**⚠️ IMPORTANTE:**
- No conectar VCC a 5V (puede dañar el OLED)
- Verificar continuidad de cables con multímetro
- Asegurarse de que no haya cruces SDA/SCL

---

### **PASO 2: Detectar dirección I2C**

Ejecuta este sketch de prueba en ambos ESP32:

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 47);  // SDA=21, SCL=47 para ESP A
                        // SDA=17, SCL=18 para ESP B
  Serial.println("\nEscaneando I2C...");
}

void loop() {
  byte error, address;
  int devices = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Dispositivo encontrado en 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }
  
  if (devices == 0)
    Serial.println("No se encontraron dispositivos I2C");
  else
    Serial.println("Escaneo completo");
  
  delay(5000);
}
```

**Resultado esperado:**
```
Dispositivo encontrado en 0x3C
```

Si aparece **0x3D** en lugar de 0x3C, cambia esta línea en el código:
```cpp
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {  // ← Cambiar dirección
```

---

### **PASO 3: Descomentar código OLED**

#### En `traffic_A.ino` y `traffic_B.ino`:

1. **Función `setup()` - Quitar comentarios:**
```cpp
// Inicializar OLED
Wire.begin(OLED_SDA, OLED_SCL);
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println("Error inicializando OLED");
} else {
  Serial.println("OLED OK");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("SEMAFORO A");  // o "SEMAFORO B"
  display.println("Iniciando...");
  display.display();
}
// Serial.println("OLED: Deshabilitado (no conectado)");  ← ELIMINAR
```

2. **Función `updateDisplay()` - Quitar `return` y descomentar:**
```cpp
void updateDisplay() {
  // OLED deshabilitado temporalmente  ← ELIMINAR
  // return;  ← ELIMINAR O COMENTAR
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // ... resto del código OLED ...
  
  display.display();
}
```

3. **Recompilar y subir código**

---

## 🧪 PRUEBA DE FUNCIONAMIENTO

Después de habilitar OLED, deberías ver:

```
Serial Monitor:
=== SEMAFORO A - Iniciando ===
OLED OK
MAC Address: A4:CF:12:9E:6B:34
Peer añadido correctamente
=== Sistema listo ===
```

Y en la pantalla OLED:
```
┌────────────────┐
│ SEMAFORO A     │
│ Estado: VERDE  │
│ Tiempo: 8 s    │
│ VEH: NO        │
│ Via B: ROJO    │
│ OK             │
└────────────────┘
```

---

## 🔍 SOLUCIÓN DE PROBLEMAS OLED

### Error: "Error inicializando OLED"
1. Verificar voltaje VCC (debe ser 3.3V, **no 5V**)
2. Revisar continuidad de SDA/SCL con multímetro
3. Probar intercambiar SDA/SCL (algunos módulos tienen etiquetas invertidas)
4. Verificar dirección I2C con scanner (puede ser 0x3D)

### OLED muestra basura/píxeles aleatorios
- Agregar resistencias pull-up 4.7kΩ en SDA y SCL a 3.3V
- Acortar cables I2C (máximo 30cm)
- Alejar cables I2C de fuentes de ruido (motores, LEDs PWM)

### OLED solo funciona a veces
- Verificar calidad de la protoboard/jumpers
- Soldar conexiones directamente al módulo OLED
- Agregar capacitor 100nF entre VCC y GND del OLED

---

## 📊 COMPARACIÓN: CON/SIN OLED

| Característica | Sin OLED | Con OLED |
|----------------|----------|----------|
| **Visibilidad local** | Solo LEDs | LEDs + texto detallado |
| **Debug** | Solo Serial Monitor | Serial + pantalla física |
| **Monitoreo remoto** | Python GUI | Python GUI + OLED |
| **Complejidad hardware** | Baja (3 pines x semáforo) | Media (+4 pines x semáforo) |
| **Velocidad loop()** | ~50ms | ~50ms (sin impacto) |
| **Confiabilidad** | Alta | Media (depende de I2C) |

**Recomendación:** Probar primero **SIN OLED** para validar lógica de control, luego agregar OLED para mejorar UX.

---

## 📝 ESTADO ACTUAL

- ✅ **Código OLED deshabilitado** en ambos ESP32
- ✅ **Sistema funcional sin pantallas**
- ⏳ **Pendiente**: Verificar hardware I2C y dirección 0x3C
- ⏳ **Pendiente**: Configurar MACs correctamente (ver `CONFIGURACION_MAC.md`)

---

## 🎯 PRÓXIMOS PASOS

1. **Subir código sin OLED** a ambos ESP32
2. **Obtener MACs reales** y configurar `peerMAC[]`
3. **Verificar comunicación ESP-NOW** en Serial Monitor
4. **Probar detección de distancia** con HC-SR04
5. **(Opcional) Habilitar OLED** siguiendo esta guía
