// Test sub-32-bit integer format conversion for CUDA surface read/write.
// UINT8 decode: zero-extend uint8 → uint32
// INT8 decode: sign-extend int8 → int32

//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj

//TEST_INPUT: RWTexture2D(format=RGBA8Uint, size=4, content = zero, mipMaps = 1):name tex_uint8
[format("rgba8ui")]
RWTexture2D<uint4> tex_uint8;

//TEST_INPUT: RWTexture2D(format=RGBA8Sint, size=4, content = zero, mipMaps = 1):name tex_int8
[format("rgba8i")]
RWTexture2D<int4> tex_int8;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Test 1: Write uint8 values and read back
    tex_uint8[int2(0, 0)] = uint4(42, 200, 0, 255);
    uint4 u0 = tex_uint8.Load(int2(0, 0));
    outputBuffer[0] = int(u0.x); // 42
    outputBuffer[1] = int(u0.y); // 200

    // Test 2: Write int8 values (including negative) and read back
    tex_int8[int2(0, 0)] = int4(42, -42, 0, 127);
    int4 i0 = tex_int8.Load(int2(0, 0));
    outputBuffer[2] = i0.x; // 42
    outputBuffer[3] = i0.y; // -42

    // Test 3: Boundary values
    outputBuffer[4] = int(u0.z); // 0
    outputBuffer[5] = int(u0.w); // 255

    outputBuffer[6] = i0.z; // 0
    outputBuffer[7] = i0.w; // 127
}
