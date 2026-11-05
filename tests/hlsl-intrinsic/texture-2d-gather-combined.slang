// A combined Sampler2D gather must read the same four texels as a separate-sampler gather of the
// same texture data at the same location, for every component (#14073).
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT: TextureSampler2D(size=4, content=gradient):name stex
Sampler2D<float4> stex;

//TEST_INPUT: Texture2D(size=4, content=gradient):name tex
Texture2D<float4> tex;

//TEST_INPUT: Sampler:name samp
SamplerState samp;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Slots 0-15 hold the combined gathers, four per component; slots 16-19 hold the per-component
// combined == separate flags, and slot 20 holds the location flag.
void store(int base, float4 combined, float4 separate)
{
    outputBuffer[base + 0] = combined.x;
    outputBuffer[base + 1] = combined.y;
    outputBuffer[base + 2] = combined.z;
    outputBuffer[base + 3] = combined.w;
    outputBuffer[16 + base / 4] = all(combined == separate) ? 1.0 : 0.0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 uv = float2(0.375, 0.625);
    store(0, stex.GatherRed(uv), tex.GatherRed(samp, uv));
    store(4, stex.GatherGreen(uv), tex.GatherGreen(samp, uv));
    store(8, stex.GatherBlue(uv), tex.GatherBlue(samp, uv));
    store(12, stex.GatherAlpha(uv), tex.GatherAlpha(samp, uv));

    // The location must matter: a gather elsewhere on the gradient differs.
    outputBuffer[20] = any(stex.GatherGreen(uv) != stex.GatherGreen(float2(0.875, 0.125))) ? 1.0 : 0.0;
}

// CUDA and Vulkan agree on these texels of the gradient texture: red, green, blue, alpha,
// then the four combined == separate flags and the location flag.
// CHECK: 0.333
// CHECK-NEXT: 0.666
// CHECK-NEXT: 0.666
// CHECK-NEXT: 0.333
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 0.666
// CHECK-NEXT: 0.666
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
