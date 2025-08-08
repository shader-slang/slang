//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj

//TEST_INPUT:ubuffer(data=[1 1 1 1], stride=4),name=inputBuffer
RWStructuredBuffer<int[2]> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void math(int data[2])
{
    // This test is to ensure that we do not break our optimization-pass which optimizes the IR of
    // `getElement(load(getElementPtr(...)), ...)` into `load(getElementPtr(getElementPtr(...), ...))`.
    //
    // This test triggers the bug given the IR: `store(outputBuffer[0], getElement(Array<>, load(getElementPtr(data,0))))`.
    // `Array<int,2>(5,6)` is not created as a local variable intentionally. We do this because we want to test the case
    // where `Array<>` must be accessed as a value (`getElement`), not a pointer (`getElementPtr`).
    //
    // If this code fails, outputBuffer[0] will contain the value `1`. We expect `6` if the bug is fixed.
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
