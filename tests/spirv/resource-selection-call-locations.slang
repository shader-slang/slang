//TEST:SIMPLE(filecheck=WRAPPED-SAMPLE): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WRAPPED-LEVEL): -target spirv-asm -stage fragment -entry main -DUNKNOWN -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PLAIN-SAMPLE): -target spirv-asm -stage fragment -entry mainControl -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WRAPPED-SAMPLE-GLSL): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WRAPPED-LEVEL-GLSL): -target glsl -stage fragment -entry main -DUNKNOWN -validate-ir -validate-ir-detailed

// A texture-containing struct argument makes type legalization rebuild the call to
// `sampleWrapped` before resource selection specializes it. The rebuilt call must keep the
// source location of the original call, so each warning points at the consumer in `main`
// instead of printing without a location. The plain-texture control never goes through
// that rebuild and must keep its located warning unchanged.
// The GLSL library expresses implicit sampling as a textual target intrinsic, so its
// conservative classification yields the unknown-requirement warning instead of the
// derivative warning SPIR-V reports for the same source.
// WRAPPED-SAMPLE-NOT: warning[E5602{{[01]}}]
// WRAPPED-SAMPLE: warning[E56020]: resource selection introduces conditional execution
// WRAPPED-SAMPLE: --> {{.*}}resource-selection-call-locations.slang:61:25
// WRAPPED-SAMPLE-NOT: warning[E5602{{[01]}}]
// WRAPPED-LEVEL-NOT: warning[E5602{{[01]}}]
// WRAPPED-LEVEL: warning[E56021]: resource selection introduces conditional execution
// WRAPPED-LEVEL: --> {{.*}}resource-selection-call-locations.slang:61:25
// WRAPPED-LEVEL-NOT: warning[E5602{{[01]}}]
// PLAIN-SAMPLE-NOT: warning[E5602{{[01]}}]
// PLAIN-SAMPLE: warning[E56020]: resource selection introduces conditional execution
// PLAIN-SAMPLE: --> {{.*}}resource-selection-call-locations.slang:67:23
// PLAIN-SAMPLE-NOT: warning[E5602{{[01]}}]
// WRAPPED-SAMPLE-GLSL-NOT: warning[E5602{{[01]}}]
// WRAPPED-SAMPLE-GLSL: warning[E56021]: resource selection introduces conditional execution
// WRAPPED-SAMPLE-GLSL: --> {{.*}}resource-selection-call-locations.slang:61:25
// WRAPPED-SAMPLE-GLSL-NOT: warning[E5602{{[01]}}]
// WRAPPED-LEVEL-GLSL-NOT: warning[E5602{{[01]}}]
// WRAPPED-LEVEL-GLSL: warning[E56021]: resource selection introduces conditional execution
// WRAPPED-LEVEL-GLSL: --> {{.*}}resource-selection-call-locations.slang:61:25
// WRAPPED-LEVEL-GLSL-NOT: warning[E5602{{[01]}}]

Texture2D texture;
SamplerState firstSampler, secondSampler;

struct TextureWrapper
{
    Texture2D texture;
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

[shader("fragment")]
float4 main(float condition : TEXCOORD) : SV_Target
{
    return sampleWrapped({ texture }, condition > 0 ? firstSampler : secondSampler);
}

[shader("fragment")]
float4 mainControl(float condition : TEXCOORD) : SV_Target
{
    return samplePlain(texture, condition > 0 ? firstSampler : secondSampler);
}
