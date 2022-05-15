#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;


out vec3 Normal;
out vec4 Position;
uniform float time;
uniform mat4 model;

float wave(vec2 pos,float amp,float dx,float dz,float frequency,float time){
    return amp * sin((pos.x*cos(dx*3.14159/180)+pos.y*sin(dz*3.14159/180))*frequency+time*10);
}
void main()
{
    
    vec4 pos = model * vec4(aPos,1.0);
    vec4 opos = model * vec4(aPos,1.0);
    pos.y += wave(pos.xz,0.25,70,70,0.25,time);
    pos.y += wave(pos.xz,3.5,90,90,0.1,time);
    pos.y += wave(pos.xz,5.5,-45,-45,0.05,-time);
    pos.y += wave(pos.xz,5,45,-45,0.06,time);
    

    //vec3 tangent = normalize(vec3(1,k*amplitude*cos(f),0));
    //Normal = mat3(transpose(inverse(model))) * vec3(-tangent.y,tangent.x,0);
    //Normal = mat3(transpose(inverse(model))) * vec3(0,1,0);

    float heightL = wave(vec2(pos.x-0.0001,pos.z),0.25,70,70,0.5,time) + wave(vec2(pos.x-0.0001,pos.z),3.5,90,90,0.2,time) + wave(vec2(pos.x-0.0001,pos.z),5.5,-45,-45,1,-time) +wave(vec2(pos.x-0.0001,pos.z),5,45,-45,0.12,time);
    float heightR = wave(vec2(pos.x+0.0001,pos.z),0.25,70,70,0.5,time) + wave(vec2(pos.x+0.0001,pos.z),3.5,90,90,0.2,time) + wave(vec2(pos.x+0.0001,pos.z),5.5,-45,-45,1,-time) +wave(vec2(pos.x+0.0001,pos.z),5,45,-45,0.12,time);
    float heightU = wave(vec2(pos.x,pos.z-0.0001),0.25,70,70,0.5,time) + wave(vec2(pos.x,pos.z-0.0001),3.5,90,90,0.2,time) + wave(vec2(pos.x,pos.z-0.0001),5.5,-45,-45,1,-time) +wave(vec2(pos.x,pos.z-0.0001),5,45,-45,0.12,time);
    float heightD = wave(vec2(pos.x,pos.z+0.0001),0.25,70,70,0.5,time) + wave(vec2(pos.x,pos.z+0.0001),3.5,90,90,0.2,time) + wave(vec2(pos.x,pos.z+0.0001),5.5,-45,-45,1,-time) +wave(vec2(pos.x,pos.z+1),5,45,-45,0.12,time);
    float dx = heightL-heightR;
    float dy = heightU-heightD;
    Normal = normalize(vec3(dx,2.0f,dy));
    
    

    //Normal = vec3(0,1,0);
    Position = gl_ModelViewProjectionMatrix * pos;
    gl_Position = Position;
    //gl_Position = projection * view * model * vec4(aPos, 1.0);
}