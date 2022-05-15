#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;
uniform mat4 model;
uniform sampler2D heighmap;
uniform sampler2D clickmap;
uniform sampler2D sinwavemap;


void main()
{
    vec3 pos = aPos;
    
    Position = vec3(model * vec4(aPos, 1.0));
    if(aNormal!=vec3(0.0f,0.0f,0.0f)){
        vec2 Coord = vec2((Position.x+100)/200 , (Position.z+100)/200);
        vec4 MapheightColor = texture2D(heighmap,Coord);
        float MapdeltaY = MapheightColor.x/255.0;
        vec4 ClickheightColor = texture2D(clickmap,Coord);
        float ClickdeltaY = ClickheightColor.x/255.0;
        vec4 SinwaveheightColor = texture2D(sinwavemap,Coord);
        float SinwavedeltaY = SinwaveheightColor.x/255.0;
        if(Coord.x<=0.01){
            SinwavedeltaY=0;
        }
        pos = vec3(aPos.x,aPos.y + (MapdeltaY*10000) + (ClickdeltaY*5000) + (SinwavedeltaY*20000) ,aPos.z);
        float unit = 1.0f/200.0f;
        float heightL = texture2D(heighmap,vec2(Coord.x-unit,Coord.y)).x
                       +texture2D(clickmap,vec2(Coord.x-unit,Coord.y)).x
                       +texture2D(sinwavemap,vec2(Coord.x-unit,Coord.y)).x;
        float heightR = texture2D(heighmap,vec2(Coord.x+unit,Coord.y)).x
                       +texture2D(clickmap,vec2(Coord.x+unit,Coord.y)).x
                       +texture2D(sinwavemap,vec2(Coord.x+unit,Coord.y)).x;
        float heightU = texture2D(heighmap,vec2(Coord.x,Coord.y+unit)).x
                       +texture2D(clickmap,vec2(Coord.x,Coord.y+unit)).x
                       +texture2D(sinwavemap,vec2(Coord.x,Coord.y+unit)).x;
        float heightD = texture2D(heighmap,vec2(Coord.x,Coord.y-unit)).x
                       +texture2D(clickmap,vec2(Coord.x,Coord.y-unit)).x
                       +texture2D(sinwavemap,vec2(Coord.x,Coord.y-unit)).x;
       float dx = abs(heightL-heightR);
       float dy = abs(heightU-heightD);
       float signDx = 1,signDy = 1;
       if(heightL<heightR){
            signDx = -1;
       }
       if(heightU-heightD){
            signDy = -1;
       }
       Normal = normalize(vec3(dx,2.0f,dy));
       Normal.x *= signDx;
       Normal.z *= signDy;
       Normal = mat3(transpose(inverse(model))) * Normal;
    }else{
       Normal = mat3(transpose(inverse(model))) * aNormal;
    }
    gl_Position = gl_ModelViewProjectionMatrix * model * vec4(pos, 1.0);
    //gl_Position = projection * view * model * vec4(aPos, 1.0);
}