// anyhit.slang.glsl
#version 460
#extension GL_NVX_raytracing : require

struct Params_0
{
    int mode_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    Params_0 gParams_0;
};

layout(binding = 1)
uniform texture2D gParams_alphaMap_0;

layout(binding = 2)
uniform sampler gParams_sampler_0;

struct SphereHitAttributes_0
{
    vec3 normal_0;
};
hitAttributeNVX SphereHitAttributes_0 _S2;

struct ShadowRay_0
{
    vec4 hitDistance_0;
};
rayPayloadInNVX ShadowRay_0 _S3;

void main()
{
    SphereHitAttributes_0 _S4 = _S2;

    if(bool(gParams_0.mode_0))
    {
        float val_0 = textureLod(
            sampler2D(gParams_alphaMap_0, gParams_sampler_0),
            _S4.normal_0.xy,
            float(0)).x;


        if(val_0 > float(0))
        {
            terminateRayNVX();
        }
        else
        {
            ignoreIntersectionNVX();
        }
    }

    return;
}

