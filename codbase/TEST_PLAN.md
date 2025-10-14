# TEST_PLAN.md — Plan de pruebas y criterios de aceptación

Objetivos de prueba:
- Verificar control individual de semáforos.
- Confirmar detección de vehículos por sensores.
- Validar comunicación ESP-NOW y sincronización.
- Probar adaptabilidad del sistema (prioridad según detección).
- Comprobar visualización en OLED.
- Evaluar estabilidad (≥30 minutos).

Escenarios de prueba:
1. Inicialización sin vehículos
   - Procedimiento: Encender ambos ESP sin vehículos.
   - Resultado esperado: ciclo normal con tiempos por defecto; nunca ambos en
     verde al mismo tiempo.

2. Detección simple (carril A)
   - Procedimiento: posicionar objeto frente al sensor A.
   - Resultado esperado: A solicita prioridad; en el siguiente ciclo A obtiene
     prioridad o extiende su verde; B permanece en rojo cuando A verde.

3. Detección simultánea
   - Procedimiento: objetos en ambos sensores.
   - Resultado esperado: prioridad resuelta por distancia; un solo lado en
     verde.

4. Pérdida de comunicación
   - Procedimiento: apagar temporalmente WiFi del B (o interrumpir alimentación)
   - Resultado esperado: A y B usarán timers locales; no habrá verde conflictivo.

5. Estabilidad prolongada
   - Procedimiento: dejar sistema en ejecución por 30+ minutos.
   - Resultado esperado: sin bloqueos, sin pérdida de sincronía observable.

Criterios de aceptación:
- No más de 1% de eventos con verde simultáneo en pruebas repetidas (objetivo 0%).
- Detección correcta en >90% de eventos simples (sensores calibrados).
- ESP-NOW intercambia paquetes periódicos sin latencias superiores a 1 s.
- OLED actualiza estado correctamente.

Logs y depuración:
- Activar salida Serial a 115200 para inspeccionar mensajes enviados/recibidos.
- Registrar distancias, decisiones de prioridad y cambios de estado.

Medición de latencia:
- Medir tiempo desde detección (sensor) hasta cambio visible en LED o mensaje
  de estado. Objetivo: <1 s en condiciones normales.

Notas:
- Repetir cada escenario varias veces y ajustar umbrales del sensor según resultados.