#version 450 
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords = aPos;
    //vec4 pos = projection * view * vec4(aPos, 1.0);
    vec4 pos = gl_ModelViewProjectionMatrix * model * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  