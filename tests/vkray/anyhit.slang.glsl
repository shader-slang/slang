// anyhit.slang.glsl
#version 460

#if USE_NV_RT
#extension GL_NV_ray_tracing : require
#define hitAttributeEXT hitAttributeNV
#define rayPayloadInEXT rayPayloadInNV
#define terminateRayEXT terminateRayNV
#define ignoreIntersectionEXT ignoreIntersectionNV
#else
#extension GL_EXT_ray_tracing : require
#endif

struct Params_0
{
    int mode_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    Params_0 _data;
} gParams_0;

layout(binding = 1)
uniform texture2D gParams_alphaMap_0;

layout(binding = 2)
uniform sampler gParams_sampler_0;

struct SphereHitAttributes_0
{
    vec3 normal_0;
};
hitAttributeEXT SphereHitAttributes_0 _S2;

struct ShadowRay_0
{
    vec4 hitDistance_0;
};
rayPayloadInEXT ShadowRay_0 _S3;

void main()
{
    if(gParams_0._data.mode_0 != 0)
    {
        float val_0 = textureLod(
            sampler2D(gParams_alphaMap_0, gParams_sampler_0),
            _S2.normal_0.xy,
            float(0)).x;


        if(val_0 > float(0))
        {
            terminateRayEXT;
        }
        else
        {
            ignoreIntersectionEXT;
        }
    }

    return;
}

