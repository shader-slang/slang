// Test case for GitHub issue #8148
// Crash when using static consts in generic interface methods with where clauses

// This test verifies that the compiler doesn't crash when processing
// generic interface methods that reference static const members in where clauses.
// The issue was in mergeCandidateParentsForHoistableInst which couldn't handle
// sibling scopes (interface scope and method generic scope).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj

interface IVector<T, int N> {}

// The key issue: an interface with static const members AND a generic method
// that references those static consts in where clauses
interface IEncoder<T, int In>
{
    static const int In;
    static const int Out;

    [BackwardDifferentiable]
    OutputVector eval<InputVector, OutputVector>(InputVector input)
        where InputVector : IVector<T, In>
        where OutputVector : IVector<T, Out>;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // Just test that the interface compiles without crashing
    // The crash would occur during compilation, not at runtime
    // CHECK: 2A
    outputBuffer[0] = 42;
}
