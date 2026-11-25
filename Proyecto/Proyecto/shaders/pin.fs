#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D uTex;  // pin.png
uniform vec4 uColor;     // Tinte del pin (blanco = colores originales)
void main() {
    vec4 c = texture(uTex, TexCoords);
    if (c.a < 0.3) discard;                  // Fuera de la silueta del pin
    FragColor = vec4(c.rgb * uColor.rgb, c.a * uColor.a);
}
