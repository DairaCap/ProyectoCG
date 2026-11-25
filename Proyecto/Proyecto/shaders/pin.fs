#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D uTex;  // pin.png (solo modo 0)
uniform int uMode;       // 0 = pin, 1 = aura en el piso
uniform float uTime;     // Segundos, para el latido del aura
uniform vec4 uColor;     // Tinte del pin / color del aura
void main() {
    if (uMode == 0) {
        vec4 c = texture(uTex, TexCoords);
        if (c.a < 0.3) discard;                  // Fuera de la silueta del pin
        FragColor = vec4(c.rgb * uColor.rgb, c.a * uColor.a);
    } else {
        float d = length(TexCoords - 0.5) * 2.0;   // 0 en el centro, 1 en el borde
        if (d > 1.0) discard;
        float aro = smoothstep(0.86, 0.92, d) * (1.0 - smoothstep(0.96, 1.0, d));
        float relleno = (1.0 - d) * 0.20;
        float latido = 0.65 + 0.35 * sin(uTime * 3.0);
        FragColor = vec4(uColor.rgb, (aro * 0.9 * latido + relleno) * uColor.a);
    }
}
