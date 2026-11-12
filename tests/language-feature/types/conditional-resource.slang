//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fragMain -stage fragment

extern const static bool testBool = true;
Conditional<Texture2D, testBool> testTexture;
Conditional<SamplerState, testBool> testSampler;

extern const static bool testFalse = false;
Conditional<Texture2D, testFalse> testMissingTexture;

// Conditional with a combined texture-sampler type (Sampler2D), which gets
// lowered to a struct containing separate texture and sampler fields.
Conditional<Sampler2D, testBool> testCombined;
Conditional<Sampler2D, testFalse> testMissingCombined;

// GLSL: uniform {{.*}}testTexture
// GLSL: uniform {{.*}}testSampler
// GLSL-NOT: testMissingTexture
// GLSL: uniform sampler2D testCombined
// GLSL-NOT: testMissingCombined

// HLSL: Texture2D{{.*}}testTexture
// HLSL: SamplerState{{.*}}testSampler
// HLSL-NOT: testMissingTexture
// HLSL: Texture2D{{.*}}testCombined
// HLSL: SamplerState{{.*}}testCombined
// HLSL-NOT: testMissingCombined

[shader("fragment")]
float4 fragMain(float2 UV : TEXCOORD) : SV_Target
{
    float4 result = float4(0);
    if (testTexture.get().hasValue && testSampler.get().hasValue)
        result += testTexture.get().value.Sample(testSampler.get().value, UV);
    if (testCombined.get().hasValue)
        result += testCombined.get().value.Sample(UV);
    return result;
}
