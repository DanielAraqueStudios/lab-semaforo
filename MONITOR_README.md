# Monitor de Semáforos Inteligentes - Guía de Usuario

Aplicación de escritorio en Python con PyQt6 para monitorear en tiempo real dos ESP32 con sistema de semáforos inteligentes y comunicación ESP-NOW.

## Características

### Visualización en Tiempo Real
- **Paneles duales:** Monitoreo simultáneo de Semáforo A y Semáforo B
- **LEDs simulados:** Visualización gráfica del estado (rojo/amarillo/verde)
- **Distancia del sensor:** Barra de progreso visual y valor numérico (HC-SR04)
- **Detección de vehículos:** Indicador SI/NO con alertas de color
- **Estado de sincronización:** Monitoreo de comunicación ESP-NOW
- **Logs separados:** Pestañas individuales para cada ESP32 + log combinado

### Información Mostrada por Semáforo
- Estado actual (VERDE, AMARILLO, ROJO, ALL RED)
- Tiempo restante en el estado
- Distancia medida por sensor ultrasónico (0-400 cm)
- Presencia de vehículo
- Prioridad solicitada
- Estado de la vía remota
- Estado de sincronización ESP-NOW
- Contadores de mensajes TX/RX
- Último mensaje transmitido

### Interfaz UI/UX
- Diseño moderno con tema oscuro (Fusion style)
- Indicadores LED con colores realistas
- Barras de progreso con códigos de color (verde/naranja/rojo)
- Logs con estilo terminal (fondo negro, texto verde)
- Timestamps precisos en milisegundos
- Controles intuitivos y responsivos

## Instalación

### Requisitos Previos
- Python 3.8 o superior
- Windows 10/11 (compatible con Linux/macOS ajustando puertos)

### Pasos de Instalación

1. **Clonar o descargar el repositorio**
   ```powershell
   cd lab-semaforo
   ```

2. **Crear entorno virtual (recomendado)**
   ```powershell
   python -m venv venv
   .\venv\Scripts\Activate.ps1
   ```

3. **Instalar dependencias**
   ```powershell
   pip install -r requirements.txt
   ```

   O manualmente:
   ```powershell
   pip install PyQt6 pyserial
   ```

## Uso

### Conectar los ESP32

1. **Cargar el firmware** en ambos ESP32:
   - `traffic_A.ino` en el primer ESP32
   - `traffic_B.ino` en el segundo ESP32

2. **Conectar ambos ESP32 por USB** a la computadora

3. **Identificar los puertos COM:**
   - Abrir Administrador de Dispositivos → Puertos (COM y LPT)
   - Anotar los puertos asignados (ej. COM3, COM5)

### Ejecutar el Monitor

```powershell
python monitor_semaforos.py
```

### Configurar la Aplicación

1. **Seleccionar puertos:**
   - Puerto COM A: seleccionar el puerto del Semáforo A
   - Puerto COM B: seleccionar el puerto del Semáforo B
   - Si no aparecen, presionar "🔄 Actualizar Puertos"

2. **Conectar:**
   - Presionar el botón "▶ Conectar"
   - La aplicación comenzará a recibir datos de ambos ESP32
   - Los LEDs y valores se actualizarán en tiempo real

3. **Monitorear:**
   - Ver estado de LEDs en los paneles superiores
   - Revisar distancias y detección de vehículos
   - Leer logs en las pestañas inferiores
   - Observar contadores de mensajes ESP-NOW

4. **Desconectar:**
   - Presionar "⏹ Desconectar" cuando termines
   - O simplemente cerrar la ventana

## Interpretación de Datos

### Estados del Semáforo
- **VERDE:** Paso permitido
- **AMARILLO:** Transición (3 segundos)
- **ROJO:** Paso bloqueado
- **ALL RED:** Ambos en rojo (seguridad, 1 segundo)

### Distancia del Sensor
- **< 50 cm:** Rojo - vehículo muy cerca
- **50-100 cm:** Naranja - zona de detección
- **> 100 cm:** Verde - sin vehículo

### Sincronización ESP-NOW
- **✅ OK:** Comunicación activa
- **❌ SIN SYNC:** Pérdida de conexión (>2 segundos sin mensajes)

### Mensajes en Logs
- **TX:** Mensaje enviado (estado, request, distancia)
- **RX:** Mensaje recibido del otro ESP32
- **-> ESTADO:** Cambio de estado del semáforo
- **MAC Address:** Dirección MAC del ESP32 (al inicio)

## Estructura de la Interfaz

```
┌─────────────────────────────────────────────────────────────────┐
│ [Puerto A ▼] [Puerto B ▼] [🔄 Actualizar] [▶ Conectar] [🗑 Limpiar] │
├──────────────────────┬──────────────────────┬──────────────────┤
│   SEMÁFORO A         │   SEMÁFORO B         │  LOGS            │
│  ┌──────────────┐    │  ┌──────────────┐    │ ┌──────────────┐ │
│  │ 🔴 🟡 🟢    │    │  │ 🔴 🟡 🟢    │    │ │ [A][B][Todo] │ │
│  └──────────────┘    │  └──────────────┘    │ └──────────────┘ │
│  Estado: VERDE       │  Estado: ROJO        │  [Logs en       │
│  Distancia: 45 cm    │  Distancia: 250 cm   │   tiempo real]  │
│  Vehículo: SÍ        │  Vehículo: NO        │                  │
│  [Barra dist ▓▓▓▓]   │  [Barra dist ░░░]    │  [Scroll auto]  │
│  TX: 125  RX: 120    │  TX: 118  RX: 126    │                  │
└──────────────────────┴──────────────────────┴──────────────────┘
```

## Solución de Problemas

### No aparecen puertos COM
- Verificar que los ESP32 estén conectados y encendidos
- Instalar drivers CH340 o CP2102 según el chip USB del ESP32
- Presionar "🔄 Actualizar Puertos"

### Error al conectar
- Cerrar Arduino IDE (libera el puerto serial)
- Verificar que los puertos seleccionados sean diferentes
- Reiniciar los ESP32

### No se actualizan los datos
- Verificar baudrate (debe ser 115200 en ambos)
- Comprobar que el firmware esté cargado correctamente
- Revisar logs para errores de comunicación

### LEDs no cambian
- Verificar que los ESP32 estén transmitiendo por serial
- Comprobar formato de mensajes (debe coincidir con el parser)
- Ver logs combinados para detectar problemas de parseo

## Personalización

### Cambiar colores del tema
Editar la función `main()` en `monitor_semaforos.py`, sección de paleta.

### Ajustar umbrales de distancia
Modificar valores en `update_distancia()` del método `SemaforoPanel`.

### Agregar más métricas
Extender el método `parse_serial_line()` con expresiones regulares adicionales.

## Capturas de Pantalla

_(Agregar capturas de la interfaz en funcionamiento)_

## Créditos

Proyecto académico - Universidad Militar Nueva Granada
Asignatura: Bluetooth y NOW

## Licencia

Uso educativo. Ver README principal del proyecto.
