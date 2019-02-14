// vk-structured-buffer-load.hlsl.glsl
#version 460
layout(row_major) uniform;
layout(row_major) buffer;
#extension GL_NV_ray_tracing : require

layout(std430, binding = 2) readonly buffer _S1 {
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

    float offsfloat_0 = ((gParamBlock_sbuf_0)._data[(int(uint(0)))]);

    _S2.PackedHitInfoA_0.y = offsfloat_0;
    
    return;
}