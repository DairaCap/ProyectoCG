#version 330 core
// Aura: un circulo tendido en el piso (un cuadro horizontal con un anillo brillante dibujado en el fragment shader).
layout (location = 0) in vec2 aPos;     // 0 a 1 dentro del cuadro
uniform mat4 projection;
uniform mat4 view;
uniform vec3 uCenter;    // Centro del aura, en el piso (m)
uniform float uRadio;    // Radio del aura (m)
out vec2 TexCoords;
void main() {
    vec3 world = uCenter + vec3((aPos.x - 0.5) * 2.0 * uRadio, 0.0, (aPos.y - 0.5) * 2.0 * uRadio);
    TexCoords = aPos;
    gl_Position = projection * view * vec4(world, 1.0);
}
