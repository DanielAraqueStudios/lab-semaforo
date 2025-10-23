# ESP-NOWClass Challenge

## Descripción

Este proyecto implementa un sistema de verificación de insumos entre dos ESP32 usando ESP-NOW:
- **Calamardo (Maestro, ESP32-S3):** Ingresa por Serial el código y cantidad recibida de cada producto y lo envía por ESP-NOW.
- **Don Cangrejo (Esclavo, ESP32-WROOM):** Recibe los datos por ESP-NOW y enciende el LED del GPIO 2 si la cantidad no corresponde con el inventario esperado.

## Hardware
- Maestro: ESP32-S3 (Calamardo)
- Esclavo: ESP32-WROOM (Don Cangrejo)
- LED conectado al GPIO 2 del esclavo

## Productos y cantidades esperadas
| Producto      | Código | Cantidad |
|--------------|--------|----------|
| Hamburguesas | HBRGR  | 50       |
| Pan de alga  | SWBRD  | 50       |
| Salsa camaron| SHRMP  | 15       |
| Pepinillos   | PICKL  | 20       |
| Tomate       | TMATO  | 20       |
| Lechuga      | LETCE  | 20       |

## Uso

### 1. Cargar sketches
- Sube `calamardo_master_s3.ino` al ESP32-S3 (maestro)
- Sube `don_cangrejo_slave_wroom.ino` al ESP32-WROOM (esclavo)

### 2. Conexión y configuración
- Asegúrate de que ambos ESP32 estén alimentados y cerca uno del otro.
- El maestro debe tener la MAC del esclavo configurada: `14:33:5C:38:D0:0C`
- El esclavo debe tener un LED conectado al GPIO 2 (o usar el LED integrado).

### 3. Envío de datos
- En el monitor serial del maestro, escribe el código y la cantidad juntos, por ejemplo:
  - `HBRGR50` (Hamburguesas, 50)
  - `LETCE18` (Lechuga, 18)
- Presiona Enter. El dato se envía por ESP-NOW al esclavo.

### 4. Recepción y alerta
- El esclavo muestra por Serial lo recibido.
- Si la cantidad NO corresponde con la tabla, el LED del GPIO 2 se enciende y aparece un mensaje de alerta.
- Si la cantidad es correcta, el LED permanece apagado.

## Notas
- El sistema es extensible: puedes agregar más productos en la tabla del esclavo.
- El código es compatible con ESP32-S3 (maestro) y ESP32-WROOM (esclavo).

---

Desarrollado para el reto ESP-NOWClass Challenge.
