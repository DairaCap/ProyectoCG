#version 330 core
// Dibuja un rectangulo 2D en pantalla (tarjeta informativa). aPos va de 0 a 1 dentro del rectangulo.
layout (location = 0) in vec2 aPos;
uniform vec4 uRect;      // x0, y0, x1, y1 en coordenadas de pantalla normalizadas (-1 a 1)
out vec2 TexCoords;
void main() {
    vec2 p = mix(uRect.xy, uRect.zw, aPos);
    gl_Position = vec4(p, 0.0, 1.0);
    TexCoords = vec2(aPos.x, 1.0 - aPos.y);   // La imagen se lee de arriba hacia abajo
}
