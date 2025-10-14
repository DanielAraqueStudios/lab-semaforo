# PROTOCOL.md — Protocolo ESP-NOW y formato de mensajes

Objetivo:
Definir un paquete compacto y reglas simples para el intercambio de estado entre
los dos ESP32 mediante ESP-NOW. Diseñado para baja latencia y simplicidad.

Estructura del paquete (10 bytes):
- uint8_t sender_id;        // 1 = SEMAFORO_A, 2 = SEMAFORO_B
- uint8_t seq;              // contador de mensajes
- uint8_t state;            // 0=ROJO,1=AMARILLO,2=VERDE
- uint8_t request;          // 0=ninguna,1=solicita prioridad
- uint16_t distance_cm;     // distancia medida en cm
- uint32_t timestamp_ms;    // millis() local (opcional para sincronía)

Total = 1 + 1 + 1 + 1 + 2 + 4 = 10 bytes

Operación:
- Cada nodo transmite este paquete periódicamente (ej. cada 200 ms).
- Cuando se detecta vehículo (distance_cm < umbral), se pone request=1.
- El receptor actualiza su vista del estado remoto y aplica reglas de decisión.

Reglas básicas:
- Si un nodo solicita prioridad y el otro no, el solicitante puede obtener la
  siguiente fase verde.
- Si ambos solicitan: comparar `distance_cm`; el menor gana. Si empate, usar
  `sender_id` como desempate determinístico.
- Antes de poner verde, el nodo envía notificación y espera una ventana corta
  (ack implícito por el paquete de estado entrante) o un timeout.

Manejo de errores y latencia:
- ESP-NOW puede perder paquetes; por eso se transmiten periódicamente.
- Si no se reciben paquetes del par en un tiempo mayor a T_lost (ej. 2 s),
  operar en modo local seguro (ciclo fijo, interlock conservador).

Ejemplo en C (estructura packed):

struct __attribute__((packed)) TrafficMsg {
  uint8_t sender_id;
  uint8_t seq;
  uint8_t state;
  uint8_t request;
  uint16_t distance_cm;
  uint32_t timestamp_ms;
};

Notas de implementación:
- Usar callback de recepción de ESP-NOW para actualizar `lastRemoteMsg`.
- Mantener `seq` para detectar paquetes antiguos o duplicados.
- Evitar dependencias de ACK explícitos; usar comportamiento idempotente y
  periodicidad.