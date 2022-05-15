#version 450
layout (location = 0) in vec3 aPos;


uniform mat4 model;


void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * model * vec4(aPos, 1.0);
}