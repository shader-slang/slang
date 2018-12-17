#version 460

#extension GL_NV_ray_tracing : require

layout(binding = 0) uniform texture2D gAlbedoMap_0;
layout(binding = 1) uniform sampler gSampler_0;

struct MaterialPayload_0
{
    vec4 albedo_0;
    vec2 uv_0;
};

callableDataInNV MaterialPayload_0 _S1;

void main()
{
    vec4 _S2 = textureLod(
        sampler2D(gAlbedoMap_0,gSampler_0),
        _S1.uv_0,
        float(0));

    _S1.albedo_0 = _S2;

    return;
}
