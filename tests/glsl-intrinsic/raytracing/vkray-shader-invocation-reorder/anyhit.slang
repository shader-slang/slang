// closesthit.slang

// Note: explicitly requesting `spirv_1_4` as part of the compiler
// options is for the benefit of the glslang pass-through path,
// where we cannot infer the required SPIR-V version from the code
// (and glslang doesn't include any such inference ability).

//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -profile glsl_460+spirv_1_4 -stage anyhit -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -allow-glsl -stage anyhit -entry main -target spirv-assembly -emit-spirv-directly

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

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
void main(
    SphereHitAttributes attributes,
    in out ShadowRay    ioPayload)
{
    if(gParams.mode != 0)
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

    uint index = 0U;
	HitTriangleVertexPosition(index);

	index = 1U;
	HitTriangleVertexPosition(index);

	index = 2U;
	HitTriangleVertexPosition(index);
}
