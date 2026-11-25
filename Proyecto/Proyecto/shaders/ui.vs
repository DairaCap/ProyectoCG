#version 330 core
// Dibuja un rectangulo 2D en pantalla (paneles, imagenes y letras de la interfaz).
// aPos va de 0 a 1 dentro del rectangulo; (0,0) = esquina inferior izquierda.
layout (location = 0) in vec2 aPos;
uniform vec4 uRect;      // x0, y0, x1, y1 en coordenadas de pantalla normalizadas (-1 a 1)
uniform vec4 uUV;        // u0, v0, u1, v1 de la textura (v0 = borde de arriba de la imagen)
out vec2 TexCoords;
void main() {
    vec2 p = mix(uRect.xy, uRect.zw, aPos);
    gl_Position = vec4(p, 0.0, 1.0);
    // La imagen se lee de arriba hacia abajo: arriba del rectangulo (aPos.y = 1) usa v0
    TexCoords = vec2(mix(uUV.x, uUV.z, aPos.x), mix(uUV.w, uUV.y, aPos.y));
}
