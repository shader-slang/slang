//TEST(compute):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -render-feature hardware-device
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -profile cs_6_6 -dx12 -use-dxil -shaderobj -render-feature hardware-device
//TEST(compute):COMPARE_COMPUTE_EX:-metal -compute -shaderobj 
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -g0

//TEST_INPUT:ubuffer(data=[0xD37A83FF], stride=4):name packedUArray
StructuredBuffer<uint8_t4_packed> packedUArray;

//TEST_INPUT:ubuffer(data=[0xDEADBEEF], stride=4):name packedSArray
StructuredBuffer<int8_t4_packed> packedSArray;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint8_t4_packed> outputBuffer;

// Test type layout works.
struct Custom {
    uint8_t4_packed packedU;
    uint3 other1;
    int8_t4_packed packedS;
    float other2;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint id = dispatchThreadID.x;

    Custom val;
    val.packedU = packedUArray[id];
    val.packedS = packedSArray[id];

    outputBuffer[id] = val.packedU;
    outputBuffer[id + 1] = uint8_t4_packed(val.packedS);
}
