//TEST:SIMPLE(filecheck=CHECK): -target glsl

// Test getCount() on unsized array inside RWStructuredBuffer for GLSL

struct Block1Data 
{
    uint array[];
};

//TEST_INPUT: set rwbuffer = ubuffer(data=[1 2 3 4 5 6], stride=4);
RWStructuredBuffer<Block1Data> rwbuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4);
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // This should get the length of the unsized array using .length()
    uint count = rwbuffer[0].array.getCount();
    
    // CHECK: .length()
    outputBuffer[0] = count;
}