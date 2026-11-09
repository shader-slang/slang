// Regression for #9011: arrays of resource-bearing parameter groups must legalize to valid SPIR-V.
// Covers the three confirmed resource types (Sampler2D, Texture2D, SamplerState) both as a
// resource-only and as a mixed bytes+resource element, plus literal vs. dynamic indexing,
// `TextureBuffer`, and a by-value copy of an indexed element.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry fragmentMain -stage fragment
// The fix lives in shared type legalization, so also guard the GLSL path (which previously
// segfaulted on this shape and is buildable without a GPU via glslang).
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry fragmentMain -stage fragment

struct OnlySampler2D
{
    Sampler2D tex;
};
ConstantBuffer<OnlySampler2D> s2dInputs[1]; // exact issue #9011 shape: literal index

struct OnlyTexture2D
{
    Texture2D t;
};
ConstantBuffer<OnlyTexture2D> texInputs[1];

struct OnlySamplerState
{
    SamplerState s;
};
ConstantBuffer<OnlySamplerState> sampInputs[1];

struct MixedSampler2D
{
    float4 tint;
    Sampler2D tex;
};
ConstantBuffer<MixedSampler2D> mixedS2dInputs[1]; // pair(ordinary, implicitDeref(special)) shape

struct MixedTexture2D
{
    float4 tint;
    Texture2D t;
};
ConstantBuffer<MixedTexture2D> mixedTexInputs[1];

struct MixedSamplerState
{
    float4 tint;
    SamplerState s;
};
ConstantBuffer<MixedSamplerState> mixedSampInputs[1];

// Multi-element array with a dynamic index (the element address is computed, not constant-folded).
ConstantBuffer<OnlySampler2D> dynInputs[4];

// `TextureBuffer`, another uniform parameter-group kind.
TextureBuffer<OnlySampler2D> tbInputs[2];

Texture2D g_tex; // for sampling with the sampler-only cases

[shader("fragment")]
float4 fragmentMain(float2 uv : UV, uint i : INDEX) : SV_Target
{
    float4 acc = s2dInputs[0].tex.Sample(uv);
    acc += texInputs[0].t.Load(int3(int2(uv), 0));
    acc += g_tex.Sample(sampInputs[0].s, uv);

    acc += mixedS2dInputs[0].tex.Sample(uv) * mixedS2dInputs[0].tint;
    acc += mixedTexInputs[0].t.Load(int3(int2(uv), 0)) * mixedTexInputs[0].tint;
    acc += g_tex.Sample(mixedSampInputs[0].s, uv) * mixedSampInputs[0].tint;

    acc += dynInputs[i].tex.Sample(uv);
    acc += tbInputs[i].tex.Sample(uv);

    // By-value copy of a dynamically indexed element (a load of the whole parameter group, then a
    // field access on the copy) — a distinct legalization path from indexing in place.
    OnlySampler2D copy = dynInputs[i];
    acc += copy.tex.Sample(uv);
    return acc;
}

// The `[1]` element count must survive on a split-out resource array (the bug collapsed the
// array), the resource-only sampler must sample, and the mixed `Texture2D` must fetch.
// CHECK-DAG: OpTypeArray %{{[A-Za-z0-9_]+}} %int_1
// CHECK-DAG: OpTypeSampledImage
// CHECK-DAG: OpImageSampleImplicitLod
// CHECK-DAG: OpImageFetch

// On GLSL the resource-bearing element survives as an array of combined samplers that is sampled.
// GLSL-DAG: uniform sampler2D {{.*}}s2dInputs_tex{{.*}}[1]
// GLSL-DAG: texture((s2dInputs_tex
