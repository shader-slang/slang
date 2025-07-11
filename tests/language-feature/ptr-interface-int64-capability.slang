//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -spirv -output-using-type

// Test to demonstrate Int64 capability usage with Ptr types in interfaces

interface IMyInterface
{
    uint getValue();
}

struct StructWithPtr : IMyInterface
{
    uint* ptr;
    
    uint getValue()
    {
        return 42;  // Simplified to avoid CPU limitations
    }
}

//TEST_INPUT: set gInterface = new StructuredBuffer<IMyInterface>[new StructWithPtr{}];
StructuredBuffer<IMyInterface> gInterface;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4) 
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT: type_conformance StructWithPtr:IMyInterface = 0

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 42
    outputBuffer[0] = gInterface[0].getValue();
}