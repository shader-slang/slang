#version 460
#extension GL_EXT_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;
layout(binding = 0)
uniform texture2D gAlbedoMap_0;

layout(binding = 1)
uniform sampler gSampler_0;

struct MaterialPayload_0
{
    vec4 albedo_0;
    vec2 uv_0;
};

callableDataInEXT MaterialPayload_0 _S1;

void main()
{
    _S1.albedo_0 = (textureLod(sampler2D(gAlbedoMap_0,gSampler_0), (_S1.uv_0), (0.0)));
    return;
}

