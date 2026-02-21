//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fragMain -stage fragment

// Test that Conditional<T, bool> works with resource types.

extern const static bool testBool = true;
Conditional<Texture2D, testBool> testTexture;
Conditional<SamplerState, testBool> testSampler;

// Test with hasValue = false
extern const static bool testFalse = false;
Conditional<Texture2D, testFalse> testMissingTexture;

// GLSL: uniform texture2D
// GLSL: uniform sampler
// HLSL: Texture2D
// HLSL: SamplerState
// HLSL-NOT: testMissingTexture

[shader("fragment")]
float4 fragMain(float2 UV : TEXCOORD) : SV_Target
{
    if (testTexture.get().hasValue)
        return testTexture.get().value.Sample(testSampler.get().value, UV);
    else
        return float4(1);
}
