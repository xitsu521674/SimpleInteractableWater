#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
out vec3 Color;

void main()
{
    Color = aColor;
    gl_Position = gl_ModelViewProjectionMatrix * model * vec4(aPos, 1.0);
}  