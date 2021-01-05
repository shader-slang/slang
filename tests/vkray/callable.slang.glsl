#version 460

#if USE_NV_RT
#extension GL_NV_ray_tracing : require
#define callableDataInEXT callableDataInNV
#define hitAttributeEXT hitAttributeNV
#define ignoreIntersectionEXT ignoreIntersectionNV
#define rayPayloadInEXT rayPayloadInNV
#define terminateRayEXT terminateRayNV
#else
#extension GL_EXT_ray_tracing : require
#endif

layout(binding = 0) uniform texture2D gAlbedoMap_0;
layout(binding = 1) uniform sampler gSampler_0;

struct MaterialPayload_0
{
    vec4 albedo_0;
    vec2 uv_0;
};

callableDataInEXT MaterialPayload_0 _S1;

void main()
{
    vec4 _S2 = textureLod(
        sampler2D(gAlbedoMap_0,gSampler_0),
        _S1.uv_0,
        float(0));

    _S1.albedo_0 = _S2;

    return;
}
