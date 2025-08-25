//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -emit-spirv-directly -compute
//TEST:SIMPLE(filecheck=SPIRV): -stage compute -entry computeMain -target spirv -fvk-use-c-layout
struct A
{
    uint64_t f0;
    uint32_t f1;
    // Padding of 4 bytes must exist in CPU layout, but not in scalar layout.
};

struct B
{
    float3 f;
};

struct C // Total size should be 40 bytes.
{
    A a;            // 16 bytes, alignment 8
    uint32_t test1; // 4 bytes, alignment 4
    B b;            // 12 bytes, alignment 4
    uint32_t test2; // 4 bytes, alignment 4
    bool c; // 1 bytes, alignment 1
    bool d; // 1 bytes, alignment 1
};

//TEST_INPUT:set cpuBuffer = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20], stride=4)
StructuredBuffer<C, CDataLayout> cpuBuffer;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // The CPU layout should be the same as ScalarDataLayout, except that the
    // size of structs is rounded up to their alignment.

    // CHECK: 5
    // CHECK: 9
    // CHECK: F
    // CHECK: 13
    outputBuffer[0] = cpuBuffer[0].test1;
    outputBuffer[1] = cpuBuffer[0].test2;
    outputBuffer[2] = cpuBuffer[1].test1;
    outputBuffer[3] = cpuBuffer[1].test2;
}

// SPIRV: OpMemberDecorate %A_c 0 Offset 0
// SPIRV: OpMemberDecorate %A_c 1 Offset 8
// SPIRV: OpMemberDecorate %B_c 0 Offset 0
// SPIRV: OpMemberDecorate %C_c 0 Offset 0
// SPIRV: OpMemberDecorate %C_c 1 Offset 16
// SPIRV: OpMemberDecorate %C_c 2 Offset 20
// SPIRV: OpMemberDecorate %C_c 3 Offset 32
// SPIRV: OpMemberDecorate %C_c 4 Offset 36
// SPIRV: OpMemberDecorate %C_c 5 Offset 37
// SPIRV: OpDecorate %_runtimearr_C_c ArrayStride 40

