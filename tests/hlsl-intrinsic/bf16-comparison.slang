//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 1
// CHECK: 0
// CHECK: 1

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(3.0);
    BFloat16 b = BFloat16(2.0);
    BFloat16 c = BFloat16(3.0);

    outputBuffer[0] = int(a == c);
    outputBuffer[1] = int(a == b);
    outputBuffer[2] = int(a != b);
    outputBuffer[3] = int(a != c);
    outputBuffer[4] = int(a > b);
    outputBuffer[5] = int(a >= c);
    outputBuffer[6] = int(a < b);
    outputBuffer[7] = int(b <= a);
}
