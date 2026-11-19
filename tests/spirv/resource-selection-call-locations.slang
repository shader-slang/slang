//TEST:SIMPLE(filecheck=WRAPPED-SAMPLE): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WRAPPED-LEVEL): -target spirv-asm -stage fragment -entry main -DUNKNOWN -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PLAIN-SAMPLE): -target spirv-asm -stage fragment -entry mainControl -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WRAPPED-TUPLE): -target spirv-asm -stage fragment -entry mainTuple -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WRAPPED-SAMPLE-GLSL): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WRAPPED-LEVEL-GLSL): -target glsl -stage fragment -entry main -DUNKNOWN -validate-ir -validate-ir-detailed

// A texture-containing struct argument makes type legalization rebuild the call to
// `sampleWrapped` before resource selection specializes it. The rebuilt call must keep the
// source location of the original call, so each wrapped variant's warning points at the
// `sampleWrapped` call site in `main` instead of printing without a location. The
// plain-texture control never goes through that rebuild and must keep its located warning
// unchanged. The tuple-result variant pins the same rule for a call whose result type
// decomposes into an out-argument: that call is deliberately outside the rebuilt call's
// `LegalVal`, and the location must still follow it. The location pins sit next to the
// anchored call sites (FileCheck
// `[[# @LINE+N]]` expressions, columns literal) so edits above cannot desynchronize them.
// The GLSL library expresses implicit sampling as a textual target intrinsic, so its
// conservative classification yields the unknown-requirement warning instead of the
// derivative warning SPIR-V reports for the same source.
// WRAPPED-SAMPLE-NOT: warning[E5602{{[01]}}]
// WRAPPED-SAMPLE: warning[E56020]: resource selection introduces conditional execution
// WRAPPED-LEVEL-NOT: warning[E5602{{[01]}}]
// WRAPPED-LEVEL: warning[E56021]: resource selection introduces conditional execution
// PLAIN-SAMPLE-NOT: warning[E5602{{[01]}}]
// PLAIN-SAMPLE: warning[E56020]: resource selection introduces conditional execution
// WRAPPED-TUPLE-NOT: warning[E5602{{[01]}}]
// WRAPPED-TUPLE: warning[E56020]: resource selection introduces conditional execution
// WRAPPED-SAMPLE-GLSL-NOT: warning[E5602{{[01]}}]
// WRAPPED-SAMPLE-GLSL: warning[E56021]: resource selection introduces conditional execution
// WRAPPED-LEVEL-GLSL-NOT: warning[E5602{{[01]}}]
// WRAPPED-LEVEL-GLSL: warning[E56021]: resource selection introduces conditional execution

Texture2D texture;
SamplerState firstSampler, secondSampler;
RWStructuredBuffer<float4> outBuf;

struct TextureWrapper
{
    Texture2D texture;
};

struct WrappedResult
{
    Texture2D t;
};

float4 sampleWrapped(TextureWrapper value, SamplerState sampler)
{
#ifdef UNKNOWN
    return abs(value.texture.SampleLevel(sampler, float2(0), 0));
#else
    return value.texture.Sample(sampler, float2(0));
#endif
}

float4 samplePlain(Texture2D value, SamplerState sampler)
{
    return value.Sample(sampler, float2(0));
}

WrappedResult sampleWrappedTuple(TextureWrapper value, SamplerState sampler)
{
    outBuf[0] = value.texture.Sample(sampler, float2(0));
    return WrappedResult(value.texture);
}

[shader("fragment")]
float4 main(float condition : TEXCOORD) : SV_Target
{
    // WRAPPED-SAMPLE: --> {{.*}}:[[# @LINE+8]]:25
    // WRAPPED-SAMPLE-NOT: warning[E5602{{[01]}}]
    // WRAPPED-LEVEL: --> {{.*}}:[[# @LINE+6]]:25
    // WRAPPED-LEVEL-NOT: warning[E5602{{[01]}}]
    // WRAPPED-SAMPLE-GLSL: --> {{.*}}:[[# @LINE+4]]:25
    // WRAPPED-SAMPLE-GLSL-NOT: warning[E5602{{[01]}}]
    // WRAPPED-LEVEL-GLSL: --> {{.*}}:[[# @LINE+2]]:25
    // WRAPPED-LEVEL-GLSL-NOT: warning[E5602{{[01]}}]
    return sampleWrapped({ texture }, condition > 0 ? firstSampler : secondSampler);
}

[shader("fragment")]
float4 mainControl(float condition : TEXCOORD) : SV_Target
{
    // PLAIN-SAMPLE: --> {{.*}}:[[# @LINE+2]]:23
    // PLAIN-SAMPLE-NOT: warning[E5602{{[01]}}]
    return samplePlain(texture, condition > 0 ? firstSampler : secondSampler);
}

[shader("fragment")]
float4 mainTuple(float condition : TEXCOORD) : SV_Target
{
    // WRAPPED-TUPLE: --> {{.*}}:[[# @LINE+2]]:41
    // WRAPPED-TUPLE-NOT: warning[E5602{{[01]}}]
    WrappedResult r = sampleWrappedTuple({ texture }, condition > 0 ? firstSampler : secondSampler);
    return r.t.Sample(firstSampler, float2(0));
}
