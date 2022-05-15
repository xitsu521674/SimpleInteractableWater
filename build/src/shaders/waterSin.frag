#version 450 core
out vec4 FragColor;

struct Light {
    //vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in  vec3 Normal;
in  vec4 Position;

uniform Light light;
uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform samplerCube tile;
uniform float lightmode;

void main()
{
    vec3 I = normalize(Position.xyz - cameraPos);
    vec3 Rfl = reflect(I, normalize(Normal));
    float ratio = 1.00 / 1.33;
    vec3 Rfr = refract(I, normalize(Normal), ratio);
    vec3 reflect_color = vec3(texture(skybox, Rfl).rgb);
    vec3 refract_color = vec3(texture(tile, Rfr).rgb);
    //FragColor = vec4(mix(refract_color,reflect_color,0.7),1.0);
    
    if(lightmode==1){
        vec3 mixColor = mix(refract_color,reflect_color,0.9);

        vec3 ambient = light.ambient * mixColor;

        vec3 norm = normalize(Normal);
        // vec3 lightDir = normalize(light.position - FragPos);
        vec3 lightDir = normalize(light.direction);  
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * mixColor;  
    
        // specular
        vec3 viewDir = normalize(cameraPos - Position.xyz);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = light.specular * spec * (255.0,255.0,255.0);  
        
        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result,1.0);

    }else{
        FragColor = vec4(mix(refract_color,reflect_color,0.7),1.0);
    }
}