#version 450

out vec4 FragColor;

struct Light {
    //vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;
in vec3 Position;
uniform vec3 cameraPos;
uniform Light light;

uniform samplerCube skybox;
uniform samplerCube tile;
uniform float lightmode;

void main()
{    
    vec3 I = normalize(Position - cameraPos);
    vec3 Rfl = reflect(I, normalize(Normal));
    float ratio = 1.00 / 1.0000001;
    vec3 Rfr = refract(I, normalize(Normal), ratio);
    vec3 reflect_color = vec3(texture(skybox, Rfl).rgb);
    vec3 refract_color = vec3(texture(tile, Rfr).rgb);

    if(lightmode==1){
        vec3 mixColor = mix(refract_color,reflect_color,0.9);

        vec3 ambient = light.ambient * mixColor;

        vec3 norm = normalize(Normal);
        // vec3 lightDir = normalize(light.position - FragPos);
        vec3 lightDir = normalize(light.direction);  
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * mixColor;  
    
        // specular
        vec3 viewDir = normalize(cameraPos - Position);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = light.specular * spec * (255.0,255.0,255.0);  
        
        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result,1.0);

    }else{
        FragColor = vec4(mix(refract_color,reflect_color,0.7),1.0);
    }
    
    //vec3 result = mixColor;
    //FragColor = vec4(,0,0,1.0);

    //FragColor = vec4(mix(refract_color,reflect_color,0.7),1.0);
    //FragColor = vec4(Normal.x*10,0,Normal.z*10,1.0f);
    // FragColor = texture2D(heighmap,aTexCoord);
}