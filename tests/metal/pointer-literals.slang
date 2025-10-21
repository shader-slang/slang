//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//METAL: {{\[\[}}kernel{{\]\]}} void computeMain
//METALLIB: define void @computeMain

// Test that pointer literals are handled correctly in Metal.

[numthreads(1,1,1)]
void computeMain()
{
    // Test pointer literals in some contexts that might generate kIROp_PtrLit.

    // Test 1: Basic pointer literal assignment
    void* ptr1 = nullptr;
    outputBuffer[0] = (ptr1 == nullptr) ? 1 : 0;
    // BUF: 1
    
    // Test 2: Pointer literal in struct
    struct TestStruct
    {
        void* ptr;
        int value;
    };

    TestStruct test = { nullptr, 42 };
    outputBuffer[1] = (test.ptr == nullptr) ? 1 : 0;
    // BUF: 1
}
