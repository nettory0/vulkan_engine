#version 460

layout( push_constant ) uniform constants
{
	 mat4 model;
} pushConstants;

layout(binding = 0, set = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;

void main() {
   mat4 mat = ubo.proj * ubo.view * pushConstants.model;
   gl_Position = mat* vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    normal=inNormal;
}
