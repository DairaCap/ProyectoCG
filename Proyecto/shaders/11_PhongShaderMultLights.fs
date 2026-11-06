#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 ex_N; 
in vec3 WorldPos;
in vec3 WorldNormal;
in vec3 vertexPosition_cameraspace;
in vec3 Normal_cameraspace;

uniform mat4 view;
uniform vec3 eye;

uniform sampler2D texture_diffuse1;
uniform samplerCube skybox;
uniform bool hasSkybox;

uniform vec4 MaterialAmbientColor;
uniform vec4 MaterialDiffuseColor;
uniform vec4 MaterialSpecularColor;
uniform float transparency;

// Tipos de material (Rúbrica de evaluación):
// 0 = OPACO (Phong mate)
// 1 = PLASTICO (Phong con punto especular blanco y nítido)
// 2 = METALICO (Especular tintado + reflexión de entorno del cubemap)
// 3 = TRASLUCIDO (Efecto Fresnel de Schlick + transparencia alpha)
uniform int materialType;
uniform float shininess;
uniform float metallic;

#define MAX_LIGHTS 10
uniform int numLights;

uniform struct Light {
   vec3  Position;
   vec3  Direction;
   vec4  Color;
   vec4  Power;
   int   alphaIndex;
   float distance;
} allLights[MAX_LIGHTS];

// Linterna / Spot Light del jugador (activable con [F])
uniform bool spotLightOn;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec4 spotLightColor;
uniform float spotInnerCutOff;
uniform float spotOuterCutOff;

vec4 ApplyLight(Light light, vec3 N, vec3 L, vec3 E, float shinVal) {
    vec4 K_a = MaterialAmbientColor * light.Color;

    float cosTheta = max(dot(N, L), 0.0);
    vec4 K_d = MaterialDiffuseColor * light.Color * cosTheta;

    vec3 R = reflect(-L, N);
    float cosAlpha = max(dot(E, R), 0.0);
    
    vec4 specColor = MaterialSpecularColor;
    if (materialType == 1) { // Plástico: brillo especular blanco puro
        specColor = vec4(1.0, 1.0, 1.0, 1.0) * length(MaterialSpecularColor.rgb);
    } else if (materialType == 2) { // Metálico: el brillo se tiñe del color difuso del metal
        specColor = MaterialDiffuseColor * 1.5;
    }
    
    vec4 K_s = specColor * light.Color * pow(cosAlpha, shinVal);

    float d = (light.distance > 0.001) ? light.distance : 10.0;
    float attenuation = 1.0 / (1.0 + 0.14 * (d / 2.0) + 0.07 * (d * d / 4.0));
    
    vec4 l_contribution = (K_a + K_d + K_s) * (light.Power * 0.1) * attenuation;
    return l_contribution;
}

void main()
{    
    vec3 n = normalize(Normal_cameraspace);
    vec3 e = normalize(-vertexPosition_cameraspace);
    
    float effectiveShininess = (shininess > 1.0) ? shininess : 32.0;
    if (materialType == 1) effectiveShininess = 96.0;
    else if (materialType == 2) effectiveShininess = 128.0;
    
    vec4 lightAcc = vec4(0.0);

    for(int i = 0; i < numLights; ++i) {
        vec3 LightPos_cam = (view * vec4(allLights[i].Position, 1.0)).xyz;
        vec3 l = normalize(LightPos_cam - vertexPosition_cameraspace);
        lightAcc += ApplyLight(allLights[i], n, l, e, effectiveShininess);
    }

    // Aportación de la Linterna / Spot Light del jugador
    if (spotLightOn) {
        vec3 spotPos_cam = (view * vec4(spotLightPos, 1.0)).xyz;
        vec3 spotDir_cam = normalize((view * vec4(spotLightDir, 0.0)).xyz);
        vec3 lightDirToFrag = normalize(spotPos_cam - vertexPosition_cameraspace);
        
        float theta = dot(-lightDirToFrag, spotDir_cam);
        float epsilon = spotInnerCutOff - spotOuterCutOff;
        float spotIntensity = clamp((theta - spotOuterCutOff) / (epsilon > 0.0001 ? epsilon : 0.0001), 0.0, 1.0);
        
        if (spotIntensity > 0.0) {
            float dist = length(spotLightPos - WorldPos);
            float spotAtten = 1.0 / (1.0 + 0.12 * dist + 0.05 * dist * dist);
            float diff = max(dot(n, lightDirToFrag), 0.0);
            vec3 r = reflect(-lightDirToFrag, n);
            float spec = pow(max(dot(e, r), 0.0), effectiveShininess);
            
            vec4 spotContrib = (MaterialDiffuseColor * diff + MaterialSpecularColor * spec) * spotLightColor * spotIntensity * spotAtten * 2.8;
            lightAcc += spotContrib;
        }
    }

    // Mapeo ambiental de cubemap (para metálicos y traslúcidos con Fresnel)
    vec4 envReflection = vec4(0.0);
    if (hasSkybox) {
        vec3 worldV = normalize(WorldPos - eye);
        vec3 worldR = reflect(worldV, normalize(WorldNormal));
        envReflection = texture(skybox, worldR);
    }

    vec4 texel = texture(texture_diffuse1, TexCoords);
    vec4 baseColor = texel * lightAcc;

    // Efecto Fresnel para materiales traslúcidos (Vidrio, Agua)
    if (materialType == 3) {
        vec3 worldV = normalize(eye - WorldPos);
        float cosTheta = max(dot(normalize(WorldNormal), worldV), 0.0);
        float F0 = 0.05;
        float fresnel = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
        
        vec4 glassTint = MaterialDiffuseColor;
        vec4 finalGlass = mix(glassTint * lightAcc, envReflection, fresnel * 0.75);
        finalGlass.a = clamp(transparency + fresnel * 0.45, 0.20, 0.95);
        FragColor = finalGlass;
        return;
    }
    // Efecto Metálico
    else if (materialType == 2) {
        vec4 metalColor = mix(baseColor, envReflection * MaterialDiffuseColor * 1.4, 0.45);
        metalColor.a = 1.0;
        FragColor = metalColor;
        return;
    }

    baseColor.a = transparency;
    FragColor = baseColor;
}
