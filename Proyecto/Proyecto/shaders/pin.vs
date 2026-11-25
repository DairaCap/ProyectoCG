#version 330 core
// Pin de un checkpoint en el mundo 3D: un cuadro vertical con la imagen pin.png que siempre mira al jugador (billboard).
layout (location = 0) in vec2 aPos;     // 0 a 1 dentro del cuadro
uniform mat4 projection;
uniform mat4 view;
uniform vec3 uCenter;    // Punta de abajo del pin (m)
uniform vec2 uSize;      // Ancho y alto del pin (m)
uniform vec3 uRight;     // Eje derecho de la camara, horizontal (para que el pin mire al jugador)
out vec2 TexCoords;
void main() {
    vec3 world = uCenter + uRight * ((aPos.x - 0.5) * uSize.x) + vec3(0.0, 1.0, 0.0) * (aPos.y * uSize.y);
    TexCoords = vec2(aPos.x, 1.0 - aPos.y);     // La imagen se lee de arriba hacia abajo
    gl_Position = projection * view * vec4(world, 1.0);
}
