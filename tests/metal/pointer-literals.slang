//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//METAL: {{\[\[}}kernel{{\]\]}} void computeMain
//METALLIB: define void @computeMain

// Test that pointer literals are handled correctly in Metal.

[numthreads(1,1,1)]
void computeMain(uniform uint* testPtr)
{
    // Test pointer literals in various contexts that might generate kIROp_PtrLit.

    // Test 1: Basic pointer literal assignment
    void* ptr1 = nullptr;
    outputBuffer[0] = (ptr1 == nullptr) ? 1 : 0;
    
    // Test 2: Pointer literal in struct
    struct TestStruct
    {
        void* ptr;
        int value;
    };

    TestStruct test = { nullptr, 42 };
    outputBuffer[1] = (test.ptr == nullptr) ? 1 : 0;

    // Test 3: Pointer literal in conditional
    bool useNull = true;
    uint* ptr2;
    if (useNull)
    {
        ptr2 = nullptr;
    }
    else
    {
        ptr2 = testPtr;
    }
    outputBuffer[2] = (ptr2 == nullptr) ? 1 : 0;

    // Test 4: Pointer literal in function-like context
    // This might trigger pointer literal generation during compilation
    outputBuffer[3] = 1;
}