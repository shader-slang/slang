// This test is disabled sine Slang doesn't support address-space specialization of nested pointers.
// Currently, Slang assumes all nested pointers are of the `UserPointer` address space.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot

// Check for a warning that this test uses experimental behavior.
//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot

struct Data
{
    int value1;
    float value2;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;
groupshared int shared;

static int* ptr;

void setPtrAddrSpace()
{
    //SPIRV: ([[# @LINE+1]]): warning 41405
    ptr = &shared;
    *ptr = 2;
}

void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    setPtrAddrSpace();
    outputBuffer[0] = *ptr;
}
// CHECK: 2
// CHECK-NEXT: 0
