// closesthit.slang
//TEST:CROSS_COMPILE: -profile sm_6_3 -stage anyhit -entry main -target spirv-assembly

struct SphereHitAttributes
{
    float3 normal;
};


struct ShadowRay
{
    float4 hitDistance;
};

struct Params
{
    Texture2D<float>    alphaMap;
    SamplerState        sampler;
    int                 mode;
}
ParameterBlock<Params> gParams;

void main(
    SphereHitAttributes attributes,
    in out ShadowRay    ioPayload)
{
    if(gParams.mode)
    {
        float val = gParams.alphaMap.SampleLevel(
            gParams.sampler,
            attributes.normal.xy, 0);
        if(val > 0)
        {
            AcceptHitAndEndSearch();
        }
        else
        {
            IgnoreHit();
        }
    }
}
