// Tests if we handle passing groupshared address-space correctly through `out` parameters.
//
// This test is disabled sine Slang doesn't support address-space specialization of nested pointers & `out`.
// Currently, Slang assumes all nested pointers are of the `UserPointer` address space.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot

// Check for a warning that this test uses experimental behavior.
//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot

// Tests if we handle passing groupshared address-space correctly through `out` parameters.
// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 2

struct Data
{
    int value1;
    int value2;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;
groupshared Data shared;

void foo(out Data* ptr, int* output)
{
    shared = Data(1, 2);
    // SPIRV: ([[# @LINE+1]]): warning 41405
    ptr = &shared;
    outputBuffer[0] = ptr->value1;
}

[numthreads(3, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    Data* data = nullptr;
    foo(data, outputBuffer);
    outputBuffer[1] = data->value1;
    outputBuffer[2] = data->value2;
}