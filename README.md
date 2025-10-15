# Semáforo Inteligente con ESP32 y ESP-NOW

Proyecto académico — Universidad Militar Nueva Granada

## Descripción

Este proyecto implementa un sistema de control semafórico inteligente para una intersección de dos vías, usando dos microcontroladores ESP32-S3. Cada ESP32 controla un semáforo de tres luces (rojo, amarillo, verde), detecta vehículos mediante un sensor de proximidad (HC-SR04 o IR), muestra el estado en una pantalla OLED y se comunica con el otro ESP32 usando el protocolo ESP-NOW para sincronizar los ciclos y evitar colisiones.

- **Adaptativo:** El sistema da prioridad al carril con mayor demanda o al que detecte primero un vehículo.
- **Visualización:** Cada ESP32 muestra el estado del semáforo y la detección de vehículos en una pantalla OLED.
- **Robusto:** Opera en modo seguro si se pierde la comunicación.

## Estructura del repositorio

```
lab-semaforo/
├── traffic_A.ino        # Código para SEMAFORO A (ESP32-S3)
├── traffic_B.ino        # Código para SEMAFORO B (ESP32-S3)
├── SETUP_B.md           # Instrucciones específicas para SEMAFORO B
├── codbase/
│   ├── README.md        # Índice de documentación
│   ├── DESIGN.md        # Diseño del sistema y máquina de estados
│   ├── PROTOCOL.md      # Protocolo ESP-NOW y formato de mensajes
│   ├── PINOUT.md        # Asignación de pines y conexiones
│   ├── TEST_PLAN.md     # Plan de pruebas y criterios de aceptación
│   ├── INSTALL.md       # Instrucciones de compilación y carga
│   ├── SETUP_A.md       # Instrucciones específicas para SEMAFORO A
│   └── traffic_A.ino    # Copia del código de SEMAFORO A
```

## Componentes principales
- 2 × ESP32-S3 DevKit
- 6 × LEDs (rojo, amarillo, verde por semáforo) + resistencias 220Ω
- 2 × sensores HC-SR04 (ultrasónico) o IR
- 2 × pantallas OLED I2C (SSD1306)
- 2 × buzzers (opcional)
- Protoboard, cables, fuente 5V/3.3V

## Instalación y uso rápido

1. **Instala las librerías necesarias:**
   - Adafruit SSD1306
   - Adafruit GFX Library
2. **Configura la placa en Arduino IDE:**
   - Board: ESP32S3 Dev Module
   - Port: (elige el COM correcto)
3. **Configura la MAC del peer:**
   - En cada sketch (`traffic_A.ino` y `traffic_B.ino`), reemplaza la variable `peerMAC[]` con la MAC del otro ESP32 (ver instrucciones en SETUP_A.md y SETUP_B.md).
4. **Carga el código en cada ESP32:**
   - `traffic_A.ino` en el primer ESP32
   - `traffic_B.ino` en el segundo ESP32
5. **Conecta el hardware según PINOUT.md**
6. **Abre el Serial Monitor (115200 baud) para depuración.**

## Documentación
- **`codbase/README.md`** — Índice de la documentación técnica
- **`codbase/DESIGN.md`** — Diseño lógico y máquina de estados
- **`codbase/PROTOCOL.md`** — Protocolo de comunicación ESP-NOW
- **`codbase/PINOUT.md`** — Asignación de pines y conexiones
- **`codbase/TEST_PLAN.md`** — Plan de pruebas y criterios de aceptación
- **`codbase/INSTALL.md`** — Instrucciones de compilación y carga
- **`codbase/SETUP_A.md`** — Guía de configuración para SEMAFORO A
- **`SETUP_B.md`** — Guía de configuración para SEMAFORO B

## Créditos y licencia

Proyecto académico para la asignatura de Bluetooth y NOW.

El uso no autorizado de su contenido, así como la reproducción total o parcial por cualquier persona o entidad, estará en contra de los derechos de autor.

---

¿Dudas o problemas? Consulta los archivos de la carpeta `codbase/` o abre un issue en el repositorio.