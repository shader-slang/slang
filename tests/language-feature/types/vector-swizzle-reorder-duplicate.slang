// vector-swizzle-reorder-duplicate.slang

// Test that swizzles can reorder (v.zyx) and duplicate (v.xxy) elements.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 v = float4(1.0, 2.0, 3.0, 4.0);

    // Reorder: zyx reverses first three elements
    float3 reversed = v.zyx;

    // CHECK: 3.0
    outputBuffer[0] = reversed.x;

    // CHECK: 2.0
    outputBuffer[1] = reversed.y;

    // CHECK: 1.0
    outputBuffer[2] = reversed.z;

    // Duplicate: xxy repeats x twice
    float3 duped = v.xxy;

    // CHECK: 1.0
    outputBuffer[3] = duped.x;

    // CHECK: 1.0
    outputBuffer[4] = duped.y;

    // CHECK: 2.0
    outputBuffer[5] = duped.z;

    // Reorder with w component: wzy
    float3 withW = v.wzy;

    // CHECK: 4.0
    outputBuffer[6] = withW.x;

    // CHECK: 3.0
    outputBuffer[7] = withW.y;

    // CHECK: 2.0
    outputBuffer[8] = withW.z;
}
