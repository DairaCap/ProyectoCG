#version 330 core
layout (location = 0) in vec2 aPos;

uniform vec2 uScale;

void main()
{
    gl_Position = vec4(aPos.x * uScale.x, aPos.y * uScale.y, 0.0, 1.0);
}
