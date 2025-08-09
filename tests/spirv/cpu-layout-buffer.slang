//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute
struct A
{
    uint64_t f0;
    uint32_t f1;
    // Padding of 4 bytes must exist in CPU layout, but not in scalar layout.
};

struct B
{
    A a;
    uint32_t test;
};

//TEST_INPUT:set cpuBuffer = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12], stride=4)
StructuredBuffer<B, CPUDataLayout> cpuBuffer;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // The CPU layout should be the same as ScalarDataLayout, except that the
    // size of structs is rounded up to their alignment.

    // CHECK: 5
    // CHECK: B
    outputBuffer[0] = cpuBuffer[0].test;
    outputBuffer[1] = cpuBuffer[1].test;
}
