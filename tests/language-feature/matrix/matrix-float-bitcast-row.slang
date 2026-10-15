// `bit_cast` of a matrix reads element `[r][c]` of the source from row `r`: the row holding a byte
// offset is the offset divided by the row stride.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name inputBuffer
RWStructuredBuffer<float> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 f = float2x2(inputBuffer[0], inputBuffer[1], inputBuffer[2], inputBuffer[3]);
    uint2x2 u = bit_cast<uint2x2>(f);
    outputBuffer[0] = u[0][0];
    outputBuffer[1] = u[0][1];
    outputBuffer[2] = u[1][0];
    outputBuffer[3] = u[1][1];
}

// CHECK: type: uint32_t
// CHECK-NEXT: 1065353216
// CHECK-NEXT: 1073741824
// CHECK-NEXT: 1077936128
// CHECK-NEXT: 1082130432
