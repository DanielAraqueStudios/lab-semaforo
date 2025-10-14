# DESIGN.md — Diseño del sistema semafórico inteligente

Resumen:
Este documento describe el diseño lógico del sistema compuesto por dos ESP32 que
coordinan dos semáforos en una intersección. Cada ESP32 controla su semáforo local
(LED rojo/amarillo/verde), lee un sensor de proximidad y muestra estado en una OLED.
La comunicación entre ESP32 usa ESP-NOW para sincronización en tiempo real.

Componentes principales:
- 2 × ESP32 (ESP32-S3 recomendado)
- 2 × módulos sensores (HC-SR04 o sensor IR)
- 2 × pantallas OLED I2C
- 6 × LEDs + resistencias

Máquina de estados local (por ESP32):
- ALL_RED: Estado seguro inicial o interbloqueo (1 s).
- GREEN: Luz verde activa. Duración dependiente de modo (normal/adaptativo).
- YELLOW: Luz amarilla intermedia (3 s).
- RED: Luz roja (espera mientras el otro hace GREEN).
- WAIT: Esperando confirmación o final de intercambio.

Ciclos y tiempos (valores por defecto):
- GREEN_NORMAL = 10 s
- YELLOW = 3 s
- ALL_RED = 1 s
- MAX_GREEN = 20 s
- BROADCAST_INTERVAL = 200 ms (envío periódico de estado por ESP-NOW)

Lógica adaptativa:
- Si el sensor detecta vehículo (distance < DETECTION_THRESHOLD_CM), se marca
  request=1 en el paquete ESP-NOW.
- Si un ESP detecta vehículo y el otro no, el detectante obtiene prioridad en el
  siguiente ciclo o extiende su verde.
- Conflictos se resuelven por distancia (menor distancia -> prioridad) o por
  desempate por ID cuando las distancias son similares.

Interlock de seguridad:
- Antes de activar GREEN, se asegura que el otro lado esté en RED.
- Transición debe respetar YELLOW y ALL_RED para evitar conflictos.

Fairness y prevención de inanición:
- Si un carril obtiene prioridad N veces consecutivas, forzar alternancia en el
  siguiente ciclo.

Notas de robustez:
- Uso de timers no bloqueantes (millis()).
- En caso de pérdida de conexión, usar ciclo seguro local y restablecer al recibir
  paquetes nuevamente.

Este documento sirve como guía para el código y las decisiones de diseño. Para el
protocolo y formato de mensajes, ver `PROTOCOL.md`.