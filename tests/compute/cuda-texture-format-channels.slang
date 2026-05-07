// Test channel count mismatch: format has fewer channels than access type.
// r8 (1 channel) accessed as float4 → pad with (0, 0, 0, 1)
// Also tests UNORM16 format conversion.

//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj

//TEST_INPUT: RWTexture2D(format=R8Unorm, size=4, content = one, mipMaps = 1):name tex_r8
[format("r8")]
RWTexture2D<float4> tex_r8;

//TEST_INPUT: RWTexture2D(format=RGBA16Unorm, size=4, content = one, mipMaps = 1):name tex_rgba16
[format("rgba16")]
RWTexture2D<float4> tex_rgba16;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Test 1: r8 read with float4 access → channel padding
    float4 v0 = tex_r8.Load(int2(0, 0));
    outputBuffer[0] = v0.x; // 1.0 (255/255)
    outputBuffer[1] = v0.y; // 0.0 (padded)
    outputBuffer[2] = v0.z; // 0.0 (padded)
    outputBuffer[3] = v0.w; // 1.0 (alpha default)

    // Test 2: Write and read-back RGBA16 UNORM
    tex_rgba16[int2(0, 0)] = float4(0.5, 0.25, 0.75, 1.0);
    float4 v1 = tex_rgba16.Load(int2(0, 0));
    // 0.5 -> fptoui(0.5*65535+0.5) = 32768 -> 32768/65535 = 0.500008
    outputBuffer[4] = v1.x;
    // 0.25 -> fptoui(0.25*65535+0.5) = 16384 -> 16384/65535 = 0.250004
    outputBuffer[5] = v1.y;

    // Test 3: r8 write with float4 access → only first channel written
    tex_r8[int2(1, 0)] = float4(0.5, 0.99, 0.99, 0.99);
    float4 v2 = tex_r8.Load(int2(1, 0));
    // Only first channel matters: 0.5 -> 128/255 = 0.501961
    outputBuffer[6] = v2.x;
    // Other channels still padded
    outputBuffer[7] = v2.w; // 1.0 (alpha default)
}
