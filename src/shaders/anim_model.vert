/*
 * Based on the LearnOpenGL "Skeletal Animation" chapter
 * (https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation,
 *  https://github.com/JoeyDeVries/LearnOpenGL)
 * Copyright (c) Joey de Vries and LearnOpenGL contributors.
 * Licensed under CC BY-NC 4.0 (https://creativecommons.org/licenses/by-nc/4.0/),
 * see third_party_licenses/CC-BY-NC-4.0.txt
 *
 * Modified by nettory0, 2026: ported to Vulkan, renamed types, integrated into the engine.
 */
#version 460


layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec4 colorIn;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;
layout(location = 6) in ivec4 boneIds; 
layout(location = 7) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

struct SMaterial
{
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};

layout( push_constant ) uniform constants
{
	 mat4 model;
} pushConstants;

layout(binding = 0, set = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 0, set = 2) uniform animatedMat {
    mat4 finalBonesMatrices[MAX_BONES];
} animated;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;
layout(location = 2) out vec3 FragPos;
layout(location = 3) out vec2 texCoord;

void main()
{

    mat4 summ=mat4(0);
    int idTemp=0;
    vec4 pos4=vec4(pos,1.0f);
    if(boneIds[0] == -1)
    summ=mat4(1);
    else
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        idTemp=boneIds[i];
        summ+=animated.finalBonesMatrices[idTemp] * weights[i];
   }
   texCoord = tex;
   color = vec3(colorIn);
   mat4 model = pushConstants.model * summ;
   FragPos = vec3(model * pos4);
   normal = mat3(transpose(inverse(model))) * norm; 
   gl_Position =  ubo.proj * ubo.view * model * pos4;
}
