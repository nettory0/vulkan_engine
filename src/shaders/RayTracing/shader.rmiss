#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#include "../common/random.glsl"
#include "../common/raytrace_struct.glsl"

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main() {
  rayPayload.radiance = vec3(0.5);
  rayPayload.exit = true;
}
