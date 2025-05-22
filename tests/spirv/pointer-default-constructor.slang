//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float* a = {};
    float* b = a + 32;

    // CHECK: 0
    outputBuffer[0] = uint(uint64_t(a));
    // CHECK: 128
    outputBuffer[1] = uint(uint64_t(b));
    // CHECK: 1
    outputBuffer[2] = uint(a == nullptr);
    // CHECK: 1
    outputBuffer[3] = uint(Ptr<int>() != Ptr<int>(0xAB64));
}
