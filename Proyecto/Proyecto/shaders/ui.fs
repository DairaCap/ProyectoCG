#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D uTex;
uniform vec4 uColor;     // Color (o tinte) con transparencia
uniform int uMode;       // 0 = color liso, 1 = imagen RGBA, 2 = letra (la textura solo trae la forma en el canal rojo)
void main() {
    if (uMode == 0)      FragColor = uColor;
    else if (uMode == 1) FragColor = texture(uTex, TexCoords) * uColor;
    else                 FragColor = vec4(uColor.rgb, uColor.a * texture(uTex, TexCoords).r);
}
