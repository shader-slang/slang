//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot

// Tests if we handle passing device address-space correctly through `out` parameters.
// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 2

struct Data
{
    int value1;
    int value2;
}

//TEST_INPUT:ubuffer(data=[1 2 0], stride=4),name=inputBuffer
uniform int* inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;

void foo(out int* ptr, int* output)
{
    ptr = &inputBuffer[0];
    outputBuffer[0] = ptr[0];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    int* data = nullptr;
    foo(data, outputBuffer);
    outputBuffer[1] = data[0];
    outputBuffer[2] = data[1];
}