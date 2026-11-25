#version 330 core
// Checkpoint en el mundo 3D. Modo 0: pin vertical que siempre mira al jugador (billboard).
// Modo 1: aura circular tendida en el piso que marca la zona de interaccion.
layout (location = 0) in vec2 aPos;     // 0 a 1 dentro del cuadro
uniform mat4 projection;
uniform mat4 view;
uniform vec3 uCenter;    // Modo 0: punta del pin. Modo 1: centro del aura en el piso
uniform vec2 uSize;      // Modo 0: ancho y alto del pin (m). Modo 1: uSize.x = diametro del aura
uniform vec3 uRight;     // Eje derecho de la camara, horizontal (para que el pin mire al jugador)
uniform int uMode;
out vec2 TexCoords;
void main() {
    vec3 world;
    if (uMode == 0) {
        world = uCenter + uRight * ((aPos.x - 0.5) * uSize.x) + vec3(0.0, 1.0, 0.0) * (aPos.y * uSize.y);
        TexCoords = vec2(aPos.x, 1.0 - aPos.y);     // La imagen se lee de arriba hacia abajo
    } else {
        world = uCenter + vec3((aPos.x - 0.5) * uSize.x, 0.0, (aPos.y - 0.5) * uSize.x);
        TexCoords = aPos;
    }
    gl_Position = projection * view * vec4(world, 1.0);
}
