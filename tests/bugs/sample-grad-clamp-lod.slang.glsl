//TEST_IGNORE_FILE:
#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_ARB_sparse_texture_clamp : require
layout(row_major) uniform;
layout(row_major) buffer;

layout(binding = 0)
uniform texture2DArray t2D_0;

layout(binding = 1)
uniform sampler samplerState_0;

struct ShadowRay_0
{
    float hitDistance_0;
};

rayPayloadInEXT ShadowRay_0 _S1;

void main()
{
    vec4 val_0 = (textureGradOffsetClampARB(sampler2DArray(t2D_0,samplerState_0), (vec3(_S1.hitDistance_0 * 0.20000000000000001110, _S1.hitDistance_0 * 0.29999999999999998890, 0.20000000000000001110)), (vec2(float(0), float(0))), (vec2(float(0), float(0))), (ivec2(0)), (0.50000000000000000000)));

    float _S2 = dot(val_0, val_0);

    _S1.hitDistance_0 = _S2;
    return;
}
