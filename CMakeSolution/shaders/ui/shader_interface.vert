#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;


layout( push_constant ) uniform constants
{
	 vec4 offset;
} pushConstants;


void main() {
   gl_Position = (vec4(inPosition.x + pushConstants.offset.x, inPosition.y+pushConstants.offset.y, 0, 1.0));
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    normal=inNormal;
}