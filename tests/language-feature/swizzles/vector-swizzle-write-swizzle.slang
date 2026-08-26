//TEST(compute):COMPARE_COMPUTE: -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE: -vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj -output-using-type

// Test that writing to swizzles of vector swizzles works correctly

// CHECK: 2.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 9.0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    float4 a = float4(0);

    a.zw.y = 9.0;
    a.xyzw.yx = float2(1.0, 2.0);

    // a = (2.0, 1.0, 0.0, 9.0)

    outputBuffer[tid] = a[tid];
}
