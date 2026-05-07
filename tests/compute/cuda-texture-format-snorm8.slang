// Test SNORM8 format conversion for CUDA surface read/write.
// SNORM8 decode: max(sitofp(raw) * (1.0/127.0), -1.0)
// SNORM8 encode: fptosi(round(clamp(val, -1, 1) * 127.0))

//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj

//TEST_INPUT: RWTexture2D(format=RGBA8Snorm, size=4, content = zero, mipMaps = 1):name tex
[format("rgba8_snorm")]
RWTexture2D<float4> tex;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Test 1: Write positive values and read back
    tex[int2(0, 0)] = float4(0.5, -0.5, 1.0, -1.0);
    float4 v0 = tex.Load(int2(0, 0));
    // 0.5 -> round(0.5 * 127) = round(63.5) = 64 -> 64/127 = 0.503937
    outputBuffer[0] = v0.x;
    // -0.5 -> round(-0.5 * 127) = round(-63.5) = -64 -> max(-64/127, -1) = -0.503937
    outputBuffer[1] = v0.y;
    // 1.0 -> round(1.0 * 127) = 127 -> 127/127 = 1.0
    outputBuffer[2] = v0.z;
    // -1.0 -> round(-1.0 * 127) = -127 -> max(-127/127, -1) = -1.0
    outputBuffer[3] = v0.w;

    // Test 2: Clamp edge cases
    tex[int2(1, 0)] = float4(1.5, -1.5, 0.0, 0.25);
    float4 v1 = tex.Load(int2(1, 0));
    // 1.5 clamped to 1.0 -> 127 -> 1.0
    outputBuffer[4] = v1.x;
    // -1.5 clamped to -1.0 -> -127 -> -1.0
    outputBuffer[5] = v1.y;
    // 0.0 -> 0 -> 0.0
    outputBuffer[6] = v1.z;
    // 0.25 -> round(0.25 * 127) = round(31.75) = 32 -> 32/127 = 0.251969
    outputBuffer[7] = v1.w;
}
