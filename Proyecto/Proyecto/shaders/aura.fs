#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform vec4 uColor;     // Color del aura (verde, rojo o crema)
uniform float uTime;     // Segundos, para el latido del brillo
void main() {
    float d = length(TexCoords - 0.5) * 2.0;     // 0 en el centro, 1 en el borde del aura
    if (d > 1.0) discard;
    float aro = smoothstep(0.80, 0.88, d) * (1.0 - smoothstep(0.95, 1.0, d));   // Anillo exterior
    float relleno = (1.0 - d) * 0.30;                                           // Resplandor suave por dentro
    float latido = 0.75 + 0.25 * sin(uTime * 3.0);
    FragColor = vec4(uColor.rgb, (aro * 0.95 * latido + relleno) * uColor.a);
}
