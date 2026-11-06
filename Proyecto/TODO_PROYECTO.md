# CONTROL Y CHECKLIST GENERAL DEL PROYECTO: "HOGAR BOTÁNICO 3D"
**Laboratorio de Computación Gráfica e Interacción Humano-Computadora (UNAM - Semestre 2027-1)**  
**Profesor:** Dr. Sergio Teodoro Vite  
**Autores:** Calzada Pérez Daira Aimé, Macías Niño Carmen Violeta, Martínez Ojeda Jonhatan Alexis, Romero Frías Maricruz Annette  

---

## RESUMEN DE CUMPLIMIENTO ACTUALIZADO
- [x] **1. Sistema de Puertas y Manijas Sincronizadas**: Manijas vinculadas y rotando solidariamente con sus puertas alrededor del eje de bisagra (`Puerta.027` a `031` con `Manija.001` a `005`). Todas las puertas abren y cierran perfectamente con sus manijas en su sitio.
- [x] **2. Orientación y Eje del Personaje Corregido**: Se identificó que Blender exportó el FBX con conversión a Y-up en el nodo padre. Se corrigió el flag de carga a `rotateZUpToYUp = false` y la posición de pies a `camera.BasePosition.y - camera.eyeHeight`, logrando que el personaje se pare erguido y firme sobre el piso (+Y) mirando hacia la dirección de avance, eliminando el fallo donde salía acostado.
- [x] **3. Animación Esquelética del Personaje al Caminar (Piernas y Brazos)**: Integración en `AvatarController` de los clips de animación horneados en `personaje_estudiante.fbx`:
  - **`Esqueleto|Caminar`**: Al presionar [W, A, S, D], se evalúa la jerarquía de huesos (`Muslo.L`, `Pierna.L`, `Pie.L` y `Muslo.R`, `Pierna.R`, `Pie.R`) produciendo un paso alternado y fluido en contra-fase, junto con el balanceo natural de los brazos.
  - **`Esqueleto|Reposo`**: Al detenerse, el avatar realiza la animación de reposo con sutil movimiento respiratorio en tórax y brazos.
  - **Accesorios adjuntos**: La gorra (`Gorra`) sigue fielmente el movimiento de la cabeza (`Cabeza`).
- [x] **4. Limpieza de Modelos Externos (En Espera de Modelos Finales)**: Siguiendo la instrucción ("Quita todos los modelos que no están en Casa_colisiones2.fbx. Todos los modelos me los van a dar después"), se retiraron todas las mallas generadas por código procedural (`BotanicalSystem` y `HouseDecorations`). La escena renderiza exclusivamente la geometría de `Casa_coliciones2.fbx` (muros, pisos, techos, puertas, ventanas, colisionadores), los interruptores de luz y el avatar del estudiante.
- [x] **5. Materiales de Rúbrica (GLSL)**: Implementados los 4 tipos requeridos: Opacos (Phong mate), Plásticos (especular blanco puro concentrado), Metálicos (especular coloreado con tinte del metal y reflexión de entorno de cubemap) y Traslúcidos (con ecuación Fresnel de Schlick y transparencia alpha).
- [x] **6. Efectos de Luz**: Multi-Luces con atenuación cuadrática real, Ciclo Solar Día/Noche procedural en tiempo real con transición de cielo y color crepuscular/lunar, Linterna frontal Spot Light del jugador (tecla [F]) con cono de corte suave, e interruptores de pared físicos que apagan/encienden las luces de cada estancia.
- [x] **7. Puntero Dinámico (Crosshair Inteligente)**: El cursor central permanece OCULTO en reposo y se activa ÚNICAMENTE cuando la mira apunta a un elemento interactivo (puertas, ventanas, interruptores de pared), con cambio dinámico de color reactivo.
- [x] **8. Físicas 3D y Navegación**: Detección de colisiones contra AABBs de la casa, soporte de gravedad vertical, salto suave con la barra espaciadora y cambio de perspectiva 1ra / 3ra persona con la tecla [P].

---

## MAPA DE ASOCIACIÓN DE PUERTAS Y MANIJAS RESUELTO
| Puerta | Nodo Malla Puerta | Nodo Malla Manija | Pivote Bisagra (X, Y, Z) | Eje de Rotación |
| :--- | :--- | :--- | :--- | :--- |
| **Principal Izq** | `Puerta.027` | `Manija.003` | `(-3.0587, 1.5020, 3.6825)` | Y (+85°) |
| **Principal Der** | `Puerta.028` | `Manija.002` | `(-0.6698, 1.5020, 3.6825)` | Y (-85°) |
| **Recámara Este** | `Puerta.029` | `Manija.005` | `( 1.7213, 1.3440, 1.3055)` | Y (+85°) |
| **Baño** | `Puerta.030` | `Manija.001` | `( 3.0146, 1.3440, 0.9984)` | Y (+85°) |
| **Pasillo / Rec. 2** | `Puerta.031` | `Manija.004` | `( 1.7213, 1.3440, -1.4759)` | Y (+85°) |

---

## GUÍA RÁPIDA DE CONTROLES EN EJECUCIÓN
*   **[W, A, S, D]**: Desplazamiento horizontal. En 3ra persona activa la animación de caminata con movimiento de piernas.
*   **[Espacio]**: Salto con física de gravedad.
*   **[P]**: Alternar entre Primera Persona y Tercera Persona (Avatar Estudiante).
*   **[Click Izquierdo] o [E]**: Interactuar con el objeto enfocado bajo el cursor (abrir/cerrar puertas, deslizar ventanas, encender/apagar interruptores).
*   **[F]**: Encender / Apagar Linterna Frontal (Spot Light).
*   **[T]**: Pausar / Reanudar el ciclo día/noche del sol.
*   **[R]**: Deslizar ventanas.
*   **[O]**: Abrir o cerrar todas las puertas a la vez.
*   **[M] / [N] / [B]**: Alternar modos de renderizado (Líneas / Sólido / Puntos).
*   **[ESC]**: Salir del programa.
