//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// Tests if we handle passing groupshared address-space correctly through `out` parameters.
// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 0

struct Data
{
    int value1;
    int value2;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;
groupshared Data shared;

void foo(out Data* ptr)
{
    shared = Data(1, 2);
    ptr = &shared;
}

[numthreads(3, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    Data* data = nullptr;
    foo(data);
    outputBuffer[0] = data->value1;
    outputBuffer[1] = data->value2;
}