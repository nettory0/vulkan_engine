#version 460

#extension GL_KHR_vulkan_glsl : enable
//#extension GL_GOOGLE_include_directive : enable
//#extension GL_ARB_shading_language_include : require

//#include "raycommon.glsl" include works  GL_ARB_shading_language_include : require || GL_GOOGLE_include_directive : enable

struct SMaterial
{
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};


layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 color;

layout(location = 2) in vec3 FragPos;
layout(location = 3) in vec2 texCoord;

layout(location = 0) out vec4 FragColor;

vec3 lightPos[3];

layout(binding = 0, set = 1) uniform sampler2D textureSampler;

int lights = 3;

void main()
{   
   SMaterial matTmp;
   matTmp.ambientColor =vec3(0.2) ;
   matTmp.diffuseColor =vec3(0.2) ;
   matTmp.specularColor =vec3(0.2) ;
   matTmp.shininess = 0.2;
   lightPos[0] = vec3(0.0f, 10.0f, 0.0f);
   lightPos[1] = vec3(5.0f, 10.0f, 5.0f);
   lightPos[2] = vec3(10.0f, 10.0f, 5.0f);
   vec3 result = vec3(0);
   for(int i =0; i < lights; i++)
   {    
   vec3 outColor1 = texture(textureSampler, texCoord).xyz;
    vec3 lightColor = vec3(1,1,1);
    vec3 viewPos = vec3(0.5, 0.5, 0.5);
  	
    vec3 ambient = matTmp.ambientColor;
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos[i] - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
     vec3 diffuse = (diff * matTmp.diffuseColor);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), matTmp.shininess);
     vec3 specular = (spec * matTmp.specularColor);  
     result += (ambient + diffuse + specular) * lightColor * outColor1 * color;//color;
    }
    FragColor = vec4(result/lights, 1.0);

}
