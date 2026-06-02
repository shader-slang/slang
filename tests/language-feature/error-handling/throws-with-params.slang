//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

// Regression test for #8742.
//
// `IRFuncType::getParamCount()` used to return an inflated count for
// throwing functions (one extra for the trailing
// `IRFuncThrowTypeAttr` operand). This in turn confused
// `slang-ir-cleanup-void` into rebuilding the `IRFuncType` and losing
// its throw attribute, corrupting the function signature for any
// throwing function with at least one parameter.
//
// Compiling and running such a function exercises both the parameter
// accessor and the void-cleanup pass; if either is wrong, this test
// either fails to compile or computes the wrong answer.

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError
{
    Fail
};

int addOrThrow(int a, int b) throws MyError
{
    if (a < 0)
        throw MyError.Fail;
    return a + b;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    do
    {
        // CHECK: 7
        outputBuffer[0] = try addOrThrow(3, 4);
    }
    catch
    {
        outputBuffer[0] = -1;
    }
}
