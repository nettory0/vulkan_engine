#version 460

#extension GL_KHR_vulkan_glsl : enable
#define LIGHT_TYPE_POINT 0        
#define LIGHT_TYPE_SPOT 1         
#define LIGHT_TYPE_DIRECTIONAL 2  
#define MAX_LIGHTS 100            

#define TEXTURE_COLOR 0           
#define TEXTURE_NORMAL 1         
#define TEXTURE_SPECULAR 2        
#define TEXTURE_DISPLACE 3       




layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 FragPos;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 1) uniform sampler2D texSampler;
//layout(binding = 0, set = 2) uniform sampler2D texSampler2;

struct LightSource
{
    vec3 position;
    float radius;
    vec3 color;
    vec3 orientation;
    float attenuationQuadratic;
    float attenuationLinear;
    float cutOffAngleCos;
    float cutOffOuterAngleCos;
    uint type;
};


struct Fragment
{
    vec3 position;     
    vec3 color;     
    vec3 normal;       
    vec3 toView;       
    float specularity;  
};

struct Material
{
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};
const int countLightSource=2;
LightSource lightSources[countLightSource];
const vec3 lightPos = vec3(0.0f, 10.0f, 0.0f);


Material materialSettings;

vec3 pointLight(Fragment f, LightSource l, Material m);
vec3 spotLight(Fragment f, LightSource l, Material m);
vec3 directionalLight(Fragment f, LightSource l, Material m);
vec3 adsSumm(Fragment f, LightSource l, Material m, vec3 directionLight);

void main() 
{
    vec3 outColor1 = texture(texSampler, fragTexCoord).xyz;
    vec3 lightColor =vec3(1,1,1);
    vec3 viewPos= vec3(0.5, 0.5, 0.5);
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * outColor1;//color;
    outColor = vec4(result, 1.0);

}

vec3 calculateDiffuseComponent(Fragment f, LightSource l, Material m, vec3 fragmentToLight)
{
    float brightness = max(dot(f.normal, fragmentToLight), 0.0);
    return brightness * l.color * m.diffuseColor * f.color;
}

vec3 calculateSpecularComponent(Fragment f, LightSource l,  Material m, vec3 fragmentToLight, bool blinnPhong)
{
    if(blinnPhong)
    {
        vec3 halfwayDir = normalize(fragmentToLight + f.toView);
        float brightness = pow(max(dot(f.normal, halfwayDir), 0.0), m.shininess);
        return brightness * l.color * m.specularColor * f.specularity;
    }
    else
    {
        vec3 reflectedLightDir = reflect(-fragmentToLight, f.normal);
        float brightness = pow(max(dot(f.toView, reflectedLightDir), 0.0), m.shininess);
        return brightness * l.color * m.specularColor * f.specularity;
    }
}

vec3 pointLight(Fragment f, LightSource l, Material m)
{
    vec3 directionLight=normalize(l.position - f.position);
    float dist = length(l.position - f.position);
    float attenuation = 1.0f / (1.0f + l.attenuationLinear * dist + l.attenuationQuadratic * (dist * dist));
    
    return attenuation * adsSumm(f, l, m, directionLight);
}

vec3 spotLight(Fragment f, LightSource l, Material m)
{
    vec3 directionLight=normalize(l.position - f.position);
    float dist = length(l.position - f.position);
    float attenuation = 1.0f / (1.0f + l.attenuationLinear * dist + l.attenuationQuadratic * (dist * dist));
    float thetaCos = dot(directionLight, normalize(-l.orientation));
    float epsilon = l.cutOffAngleCos - l.cutOffOuterAngleCos;
    float intensity = clamp((thetaCos - l.cutOffOuterAngleCos) / epsilon, 0.0, 1.0);
    
    return attenuation * intensity * adsSumm(f, l, m, directionLight);
}

vec3 directionalLight(Fragment f, LightSource l, Material m)
{
    vec3 directionLight=normalize(l.orientation);
    return adsSumm(f, l, m, directionLight);
}

vec3 adsSumm(Fragment f, LightSource l, Material m, vec3 directionLight)
{
    vec3 ambient = l.color * m.ambientColor;
    vec3 diffuse = calculateDiffuseComponent(f, l, m, directionLight);
    vec3 specular = calculateSpecularComponent(f, l, m, directionLight, false);
 
    return diffuse + specular + ambient;
}

float depthMap(vec2 uv, bool inverse)
{
    return 1;//inverse ? 1 - texture(_textures[TEXTURE_DISPLACE], uv).r : texture(_textures[TEXTURE_DISPLACE], uv).r;
}

vec2 paralaxMappedUv(vec2 uv, vec3 toView, bool inverseMap)
{
    vec3 toViewT=vec3(1,1,1);// = normalize(fs_in.tbnMatrixInverse * toView);
    const float minDepthLayers = 8;
    const float maxDepthLayers = 16;
    float depthLayers = mix(maxDepthLayers,minDepthLayers,abs(dot(vec3(0.0, 0.0, 1.0), toViewT)));
    float layerDepth = 1.0f / depthLayers;
    vec2 p = toViewT.xy * 0.15f;
    vec2 layerUvOffset = p / depthLayers;
    vec2 currentUv = uv;
    float currentDepthCalculated = 0.0f;
    float currentDepthFromMap = depthMap(currentUv,inverseMap);
    while(currentDepthCalculated < currentDepthFromMap)
    {
        currentUv -= layerUvOffset;
        currentDepthCalculated += layerDepth;
        currentDepthFromMap = depthMap(currentUv,inverseMap);
    }
    vec2 previousUv = currentUv + layerUvOffset;
    float previousDepthCalculated = currentDepthCalculated - layerDepth;
    float currentDeptDelta = currentDepthFromMap - currentDepthCalculated;
    float beforeDepthDelta = depthMap(previousUv,inverseMap) - previousDepthCalculated;

    float weight = currentDeptDelta / (currentDeptDelta - beforeDepthDelta);
    vec2 finalUv = (previousUv * weight) + (currentUv * (1.0 - weight));

    return finalUv;
}
