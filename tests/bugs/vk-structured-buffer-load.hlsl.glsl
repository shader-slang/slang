// vk-structured-buffer-load.hlsl.glsl
//TEST_IGNORE_FILE:

#version 460
layout(row_major) uniform;
layout(row_major) buffer;
#extension GL_NV_ray_tracing : require

#define rcp_tmp _S2
#define RayData _S3
#define Attributes _S4

#define tmpA _S5
#define tmpB _S6
#define tmpC _S7

layout(std430, binding = 1) readonly buffer _S1 {
    float _data[];
} gParamBlock_sbuf_0;

float rcp_0(float x_0)
{
    float rcp_tmp = float(1.00000000000000000000) / x_0;
    return rcp_tmp;
}

struct RayHitInfoPacked_0
{
    vec4 PackedHitInfoA_0;
};

rayPayloadInNV RayHitInfoPacked_0 RayData;

struct BuiltInTriangleIntersectionAttributes_0
{
    vec2 barycentrics_0;
};

hitAttributeNV BuiltInTriangleIntersectionAttributes_0 Attributes;

void main()
{
    float HitT_0 = (gl_RayTmaxNV);
    RayData.PackedHitInfoA_0.x = HitT_0;

    const uint use_rcp_0 = uint(0);

    float offsfloat_0 = ((gParamBlock_sbuf_0)._data[(int(uint(0)))]);

    uint use_rcp_1 = use_rcp_0|uint(HitT_0 > 0.00000000000000000000);

    if(bool(use_rcp_1))
    {

        float tmpA = rcp_0(offsfloat_0);

        RayData.PackedHitInfoA_0.y = tmpA;

    }
    else
    {

        if(use_rcp_1 > uint(0)&&offsfloat_0 == 0.00000000000000000000)
        {

            float tmpB = (inversesqrt((offsfloat_0 + 1.00000000000000000000)));

            RayData.PackedHitInfoA_0.y = tmpB;

        }
        else
        {
            float tmpC = (inversesqrt((offsfloat_0)));

            RayData.PackedHitInfoA_0.y = tmpC;

        }

    }

    return;
}
