# Muebles, plantas y objetos (Romero)

Contenido de esta carpeta (generado desde Blender con `exportar_muebles.py`):
- `Muebles.fbx` + `Muebles.fbm/`: 93 modelos (muebles, plantas, lámparas, electrodomésticos). Sin paredes ni interruptores.
  Mismos ejes que Casa_coliciones3.fbx (Z arriba, se carga con `Model(path, true)`).
- `animaciones.csv`: piezas que se mueven (pivote, eje, valor final) en ejes de OpenGL.
- `grupos.csv`: caja de cada mueble animado (para apuntarle con la mira).
- `colisiones.csv`: cajas de los muebles que bloquean el paso.

Se carga con `include/Muebles.h` desde `main.cpp` (busca `muebles/` junto al ejecutable o `../muebles`).
Interactuar: apuntar y Click o E (clóset, cajones, refri, microondas, estufa, laptop, burós, fregadero).
