# Contexto de Desarrollo - Proyecto "Hogar Botánico 3D" (OpenGL)

## 1. Estado Actual de los Assets 3D
El archivo definitivo para importar es **`Casa_coliciones_2.fbx`**. Este archivo ha sido validado y cumple con los siguientes requerimientos técnicos para el pipeline:
*   **Geometría separada:** Muros, pisos, techos, puertas, y ventanas están separados en nodos individuales (`ModelS Mesh`).
*   **Pivotes conservados:** Los objetos interactivos (puertas `Puerta.027` a `031`, ventanas) conservan sus transformaciones locales originales (traslación y rotación). Sus orígenes están correctamente ubicados en las bisagras/ejes para poder aplicar matrices de rotación en C++ sin alterar su órbita.
*   **Escala aplicada:** Todos los objetos tienen su escala aplicada (1.0, 1.0, 1.0) para evitar deformaciones en las sombras de Phong y errores en los cálculos de los límites (Bounds).

## 2. Sistema de Colisiones AABB
Se implementó un sistema de colisiones basado en geometría invisible:
*   El archivo FBX contiene 18 mallas primitivas (cubos) que envuelven las paredes y estructuras físicas.
*   Todos estos cubos tienen asignado el material **`Mat_colision`**.
*   **Instrucción de Carga (Assimp):** Durante la iteración `scene->mMeshes`, el código debe evaluar el nombre del material (`AI_MATKEY_NAME`). Si el material es `Mat_colision`, **NO** se deben generar buffers (VAO/VBO) ni enviarse al render queue. En su lugar, se deben iterar sus vértices para calcular los vectores `Min` y `Max` y almacenar el resultado en un arreglo de estructuras `AABB`.

## 3. Actualizaciones a la Arquitectura (Nuevas Mecánicas)
Se han añadido mecánicas que modifican el plan original del anteproyecto. El código de la cámara/jugador (`Player.cpp` o similar) debe contemplar lo siguiente:
*   **Salto y Gravedad:** La cámara ya no se restringe estáticamente al plano XZ. Se debe implementar física básica:
    *   Variables necesarias: `float velocidadY = 0.0f;`, `float gravedad = -9.81f;`, `bool enSuelo = true;`.
    *   Las colisiones AABB ahora deben evaluar también el eje Y (para detectar el piso al caer y el techo al saltar).
*   **Head Bobbing (Animación Procedural):** Se debe implementar un balanceo de cámara al caminar para dar realismo. Esta mecánica cuenta como una de las 5 animaciones procedurales requeridas por la rúbrica.
    *   Fórmula sugerida en el `Update()`: Aplicar una onda senoidal basada en `glfwGetTime()` a la posición Y de la cámara, activa únicamente mientras el jugador presiona las teclas de movimiento (W, A, S, D).

## 4. Tareas Inmediatas para el Desarrollo en C++
1.  **Carga de Modelo:** Integrar Assimp, cargar `Casa_coliciones_2.fbx` y separar la geometría visible de las cajas de colisión AABB (`Mat_colision`).
2.  **Física y Colisión:** Programar la detección de colisiones AABB 3D entre la cámara (o el avatar) y el vector de cajas `AABB`. Implementar la gravedad y el salto.
3.  **Animación Procedural:** Integrar el *Head Bobbing* dependiente del movimiento horizontal.
4.  **Sistema de Interacción (Picking):** Asignar un `IDforPicking` único a las mallas de las puertas e interruptores y configurar el render off-screen de 24-bits para detectar clics del usuario.