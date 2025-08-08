//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj

//TEST_INPUT:ubuffer(data=[1 1 1 1], stride=4),name=inputBuffer
RWStructuredBuffer<int[2]> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void math(int data[2])
{

    // We can change the following: `getElement(load(getElementPtr(...)), ...)` into `load(getElementPtr(getElementPtr(...)))`
    // This is done to enable future optimizations through a consistent IR form.
    //
    // Test that we do not break the alternative form via our optimization-pass: `getElement(...,load(getElementPtr(...)))`
    // Expected IR: store(outputBuffer[0], getElement(Array<>, load(getElementPtr(data,0))))
    //
    // Note: `Array<int,2>(5,6)` is not created as a local variable intentionally.
    // We do this because we want to test the case where `Array<>` must be accessed
    // as a value (`getElement`), not a pointer (`getElementPtr`).
    outputBuffer[0] = Array<int,2>(5,6)[data[0]];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(int3 id: SV_DispatchThreadID)
{
    math(inputBuffer[0]);
}

//CHECK: 6
//CHECK-NEXT: 0
