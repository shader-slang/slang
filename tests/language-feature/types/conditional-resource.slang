//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fragMain2 -stage fragment

// Test that Conditional<T, bool> works with resource types.

// GLSL: uniform sampler2D testSampler_storage_0
extern const static bool testBool = true;
Conditional<Sampler2D, testBool> testSampler;

[shader("fragment")]
float4 fragMain(float2 UV : TEXCOORD) : SV_Target
{
    if (testSampler.get().hasValue)
        return testSampler.get().value.Sample(UV);
    else
        return float4(1);
}

// HLSL: Texture2D
Conditional<Texture2D, testBool> testTexture;
SamplerState s;

// Test with hasValue = false
extern const static bool testFalse = false;
Conditional<Texture2D, testFalse> testMissingTexture;

[shader("fragment")]
float4 fragMain2(float2 UV : TEXCOORD) : SV_Target
{
    if (testTexture.get().hasValue)
        return testTexture.get().value.Sample(s, UV);
    else
        return float4(1);
}
