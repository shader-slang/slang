// Test UNORM8 format conversion for CUDA surface read/write.

//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj

//TEST_INPUT: RWTexture2D(format=RGBA8Unorm, size=4, content = one, mipMaps = 1):name tex_rgba8
[format("rgba8")]
RWTexture2D<float4> tex_rgba8;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Test 1: Read RGBA8 texture initialized to 1.0 (= 255/255)
    float4 v0 = tex_rgba8.Load(int2(0, 0));
    outputBuffer[0] = v0.x; // 1.0

    // Test 2: Write and read-back RGBA8
    tex_rgba8[int2(1, 0)] = float4(0.5, 0.25, 0.75, 1.0);
    float4 v1 = tex_rgba8.Load(int2(1, 0));
    // 0.5 -> 128/255 = 0.501961
    outputBuffer[1] = v1.x;
    // 0.25 -> 64/255 = 0.250980
    outputBuffer[2] = v1.y;

    // Test 3: Saturate edge case
    tex_rgba8[int2(2, 0)] = float4(1.5, -0.5, 0.0, 1.0);
    float4 v2 = tex_rgba8.Load(int2(2, 0));
    outputBuffer[3] = v2.x; // clamped to 1.0
    outputBuffer[4] = v2.y; // clamped to 0.0

    // Test 4: Read another channel from round-trip
    outputBuffer[5] = v1.z; // 0.75 -> 191/255 = 0.749020

    // Test 5: Alpha channel
    outputBuffer[6] = v1.w; // 1.0
    outputBuffer[7] = v2.w; // 1.0
}
