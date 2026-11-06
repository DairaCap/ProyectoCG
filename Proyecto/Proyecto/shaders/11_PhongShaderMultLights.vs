#version 330 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  tangent;
layout (location = 4) in vec3  bitangent;

out vec2 TexCoords;
out vec3 ex_N;
out vec3 WorldPos;
out vec3 WorldNormal;
out vec3 vertexPosition_cameraspace;
out vec3 Normal_cameraspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos4 = model * vec4(aPos, 1.0f);
    WorldPos = worldPos4.xyz;
    gl_Position = projection * view * worldPos4;

    TexCoords = aTexCoords;  
    
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    WorldNormal = normalize(normalMatrix * aNormal);

    vertexPosition_cameraspace = (view * worldPos4).xyz;
    Normal_cameraspace = normalize((view * vec4(WorldNormal, 0.0f)).xyz);

    ex_N = aNormal;
}
