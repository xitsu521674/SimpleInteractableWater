#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
out vec3 Normal;
out vec3 Position;
uniform int sinwavemode;
uniform float time;
float wave(vec2 pos,float amp,float dx,float dz,float frequency,float time){
    return amp * sin((pos.x*cos(dx*3.14159/180)+pos.y*sin(dz*3.14159/180))*frequency+time*10);
}
void main()
{
    vec4 pos = model * vec4(aPos, 1.0);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    if(sinwavemode==1&&aNormal!=vec3(0.0f,0.0f,0.0f)){
       float heightL = wave(vec2(pos.x-0.0001,pos.z),0.25,70,70,0.5,time) + wave(vec2(pos.x-0.0001,pos.z),3.5,90,90,0.2,time) + wave(vec2(pos.x-0.0001,pos.z),5.5,-45,-45,1,time) +wave(vec2(pos.x-0.0001,pos.z),5,45,-45,0.12,time);
       float heightR = wave(vec2(pos.x+0.0001,pos.z),0.25,70,70,0.5,time) + wave(vec2(pos.x+0.0001,pos.z),3.5,90,90,0.2,time) + wave(vec2(pos.x+0.0001,pos.z),5.5,-45,-45,1,time) +wave(vec2(pos.x+0.0001,pos.z),5,45,-45,0.12,time);
       float heightU = wave(vec2(pos.x,pos.z-0.0001),0.25,70,70,0.5,time) + wave(vec2(pos.x,pos.z-0.0001),3.5,90,90,0.2,time) + wave(vec2(pos.x,pos.z-0.0001),5.5,-45,-45,1,time) +wave(vec2(pos.x,pos.z-0.0001),5,45,-45,0.12,time);
       float heightD = wave(vec2(pos.x,pos.z+0.0001),0.25,70,70,0.5,time) + wave(vec2(pos.x,pos.z+0.0001),3.5,90,90,0.2,time) + wave(vec2(pos.x,pos.z+0.0001),5.5,-45,-45,1,time) +wave(vec2(pos.x,pos.z+1),5,45,-45,0.12,time);
       if((heightD<pos.y)||(heightU<pos.y)||(heightL<pos.y)||(heightR<pos.y)){
            Normal = vec3(0,1,0);
            
       }else{
            float dx = heightL-heightR;
            float dy = heightU-heightD;
    
            Normal = normalize(vec3(dx,1.0f,dy));
       }
       
    }
    Position = vec3(model * vec4(aPos, 1.0));
    gl_Position = gl_ModelViewProjectionMatrix * model * vec4(aPos, 1.0);
    //gl_Position = projection * view * model * vec4(aPos, 1.0);
    
}