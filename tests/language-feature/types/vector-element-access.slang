// vector-element-access.slang

// Test accessing vector elements by subscript index and by swizzle (xyzw, rgba).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 v = float4(10.0, 20.0, 30.0, 40.0);

    // Access by subscript index
    // CHECK: 10.0
    outputBuffer[0] = v[0];

    // CHECK: 20.0
    outputBuffer[1] = v[1];

    // CHECK: 30.0
    outputBuffer[2] = v[2];

    // CHECK: 40.0
    outputBuffer[3] = v[3];

    // Access by xyzw swizzle
    // CHECK: 10.0
    outputBuffer[4] = v.x;

    // CHECK: 20.0
    outputBuffer[5] = v.y;

    // Access by rgba swizzle
    // CHECK: 30.0
    outputBuffer[6] = v.b;

    // CHECK: 40.0
    outputBuffer[7] = v.a;
}
