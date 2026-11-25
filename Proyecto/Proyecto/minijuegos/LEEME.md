# Minijuegos y checkpoints

Carpeta de recursos de los minijuegos. Se empaqueta junto al ejecutable (`empaquetar.bat`).

- `pin.png`: imagen del pin de los checkpoints (siempre mira al jugador).
- `trivia/qNN_pregunta.png` y `trivia/qNN_respuesta.png`: 10 preguntas (NN = 01 a 10), 1280 x 720.

## Cómo funciona la trivia

1. En el centro de la casa hay un checkpoint (un pin rojo flotando).
2. De 6 m a 1.5 m, y mirándolo, sale el aviso "Acércate al checkpoint para interactuar".
3. Al acercarse a 1.5 m se abre el popup a la derecha. ENTER o E comienzan.
4. Se sortean 5 de las 10 preguntas. Se contesta con **A, B o C** (el movimiento y la cámara se bloquean).
5. Después de cada respuesta sale "Correcto / Incorrecto" y la imagen con la respuesta; ENTER pasa a la siguiente.
6. Al final: marcador, tiempo total y resultado de cada pregunta. ENTER cierra, R juega de nuevo, ESC sale en cualquier momento.

El cronómetro solo corre mientras se ve la pregunta (leer la respuesta no cuenta).

## Dónde cambiar cosas

- **Posición del checkpoint:** `CHECKPOINT_TRIVIA_POS` al inicio de `main.cpp` (x, y del piso, z). Si ahí hay un mueble o pared, se corre al lugar libre más cercano; la posición final sale en la consola (`[Checkpoint] Trivia en ...`).
- **Radios (aviso y apertura de la trivia):** `checkpoints.Agregar("trivia", pos, 1.5f, 6.0f)` en `Start()`.
- **Agregar o cambiar preguntas:** poner las imágenes `qNN_...png` en `trivia/` y actualizar en `include/Trivia.h` `TOTAL`, `CORRECTA` (letra correcta de cada pregunta, en orden) y `TEMA`.
- **Más checkpoints** (por ejemplo el del patio): `checkpoints.Agregar("id", posicion)`; se dibujan solos.

## Minijuego 2: Acomoda las plantas (checkpoint del patio)

- **Cómo se juega:** al acercarse al pin del patio (jardín norte) se abre la pantalla de reglas; se elige el nivel con **1, 2 o 3**. Se sortean **8 de las 15 plantas** con tarjeta y se mueven al patio. **Mantener el clic izquierdo** agarra la planta a la que apunta la mira (flota frente al jugador); al soltar el clic queda en el piso. **ENTER** califica: aura **verde** = bien, **roja** = mal (se pueden volver a agarrar). Termina cuando todas están en verde (tiempo y número de calificaciones). **ESC** sale y las plantas regresan a su lugar; **R** (al final) juega de nuevo.
- **Niveles:** 1 Básico (aura crema en el lugar correcto + zona + distancia y dirección), 2 Intermedio (solo la zona), 3 Experto (sin pistas).
- **Bien colocada:** a menos de `TOLERANCIA` (0.5 m) del lugar original en el plano (la altura no cuenta). El lugar correcto sale de la caja de cada planta en `muebles/plantas.csv`.
- **Posición del checkpoint:** `CHECKPOINT_PLANTAS_POS` en `main.cpp`.
- **Zonas que se muestran como pista:** tabla `INFO` en `include/JuegoPlantas.h` (**revisar que cada nombre de zona coincida con el plano**).
- **Cantidad de plantas:** `POR_JUEGO` en `JuegoPlantas.h`.
- **Piezas nuevas:** `JuegoPlantas.h` (lógica y pantallas), `Aura.h` + `shaders/aura.*` (círculos del piso), `PuntoLibre.h` (lugares libres), y en `Muebles.h` `plantaMatriz`/`plantaMovida` para mover plantas completas.
