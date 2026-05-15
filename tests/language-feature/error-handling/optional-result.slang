// This test is only run with the LLVM target because LLVM optimizes
// uninitialized values very aggressively and is therefore much more reliable
// at surfacing this bug.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum FooError
{
    Foo
}

Optional<int*> A() throws FooError
{
    return Ptr<int>(42);
}

int* B() throws FooError
{
    if (let src = try A())
        return src;
    return nullptr;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = 0;
    do
    {
        int* b = try B();
        // CHECK: 2A
        outputBuffer[0] = int(uintptr_t(b));
    }
    catch
    {
        outputBuffer[0] = 2;
    }
}
