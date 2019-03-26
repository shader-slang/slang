// vk-structured-buffer-load.hlsl.glsl
//TEST_IGNORE_FILE:

#version 460
layout(row_major) uniform;
layout(row_major) buffer;
#extension GL_NV_ray_tracing : require

layout(std430, binding = 1) readonly buffer _S1 {
    float _data[];
} gParamBlock_sbuf_0;

struct RayHitInfoPacked_0
{
    vec4 PackedHitInfoA_0;
};

rayPayloadInNV RayHitInfoPacked_0 _S2;

struct BuiltInTriangleIntersectionAttributes_0
{
    vec2 barycentrics_0;
};

hitAttributeNV BuiltInTriangleIntersectionAttributes_0 _S3;

void main()
{
    float HitT_0 = (gl_HitTNV);
    _S2.PackedHitInfoA_0.x = HitT_0;

    const uint use_rcp_0 = uint(0);

    float offsfloat_0 = ((gParamBlock_sbuf_0)._data[(int(uint(0)))]);



    if(bool(use_rcp_0 | uint(HitT_0 > 0.00000000000000000000)))
    {

        float _S4 = (1.0/((offsfloat_0)));

        _S2.PackedHitInfoA_0.y = _S4;

    }
    else
    {
        float _S5 = (inversesqrt((offsfloat_0)));

        _S2.PackedHitInfoA_0.y = _S5;

    }

    return;
}
