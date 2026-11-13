//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute

// Regression test for https://github.com/shader-slang/slang/issues/14378
// A `static const` global may alias a resource or sampler shader parameter, either
// directly, through a struct or array made only of resources and samplers, or by indexing
// an array of them.
// Such an initializer is not a runtime value and must not be diagnosed with E31226.

// SPIRV-NOT: E31226
// SPIRV: OpEntryPoint
// HLSL-NOT: E31226
// HLSL: computeMain
// CUDA-NOT: E31226
// CUDA: computeMain

Texture2D tex;
Texture2D tex2;
Texture2D texParams[2];
SamplerState samp;
RWStructuredBuffer<float4> outputBuffer;

struct TexSampler
{
    Texture2D t;
    SamplerState s;
}

struct Params
{
    Texture2D t;
}
ParameterBlock<Params> pb;

static const Texture2D aliasTex = tex;
static const SamplerState aliasSamp = samp;
static const TexSampler s0 = { tex, samp };
static const Texture2D texArray[2] = { tex, tex2 };
static const Texture2D aliasPbTex = pb.t;
static const Texture2D aliasElement = texParams[1];

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 uv = float2(0.5, 0.5);
    outputBuffer[0] = aliasTex.SampleLevel(aliasSamp, uv, 0);
    outputBuffer[1] = s0.t.SampleLevel(s0.s, uv, 0);
    outputBuffer[2] = texArray[1].SampleLevel(samp, uv, 0);
    outputBuffer[3] = aliasPbTex.SampleLevel(samp, uv, 0);
    outputBuffer[4] = aliasElement.SampleLevel(samp, uv, 0);
}
