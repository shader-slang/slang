//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct EmptyStruct
{
}

struct StructWithEmpty
{
    EmptyStruct empty;
    int value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    StructWithEmpty data;
    data.value = 42;
    
    // Test that empty struct doesn't break layout
    // The value should be accessible correctly
    // CHECK: 2A
    outputBuffer[0] = data.value;
}