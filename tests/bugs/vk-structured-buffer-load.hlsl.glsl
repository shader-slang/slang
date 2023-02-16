// vk-structured-buffer-load.hlsl.glsl
//TEST_IGNORE_FILE:

#version 460
#extension GL_NV_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;

layout(std430, binding = 1) readonly buffer _S1 {
    float _data[];
} gParamBlock_sbuf_0;

float rcp_0(float x_0)
{
    float _S2 = 1.0 / x_0;
    return _S2;
}

struct RayHitInfoPacked_0
{
    vec4 PackedHitInfoA_0;
};

rayPayloadInNV RayHitInfoPacked_0 _S3;

struct BuiltInTriangleIntersectionAttributes_0
{
    vec2 barycentrics_0;
};

hitAttributeNV BuiltInTriangleIntersectionAttributes_0 _S4;

void main()
{
    float HitT_0 = ((gl_RayTmaxNV));
    _S3.PackedHitInfoA_0.x = HitT_0;

    float offsfloat_0 = ((gParamBlock_sbuf_0)._data[(0)]);

    uint use_rcp_0 = 0U | uint(HitT_0 > 0.0);

    if(use_rcp_0 != 0U)
    {

        float _S5 = rcp_0(offsfloat_0);

        _S3.PackedHitInfoA_0.y = _S5;

    }
    else
    {

        if(use_rcp_0 > 0U&&offsfloat_0 == 0.0)
        {

            float _S6 = (inversesqrt((offsfloat_0 + 1.0)));

            _S3.PackedHitInfoA_0.y = _S6;

        }
        else
        {
            float _S7 = (inversesqrt((offsfloat_0)));

            _S3.PackedHitInfoA_0.y = _S7;

        }

    }

    return;
}
