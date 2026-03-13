// Null comparison on a pointer to interface type compiles correctly. The
// IFoo* can be initialized to nullptr and compared against nullptr for safe
// null-guarded dispatch patterns.
//
// The dispatch through p->getVal() is not tested here because SIMPLE tests
// don't load type conformances. The null branch path is the verification target.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    int getVal();
}

struct Foo : IFoo
{
    int val;
    int getVal() { return val; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IFoo* p = nullptr;

    // Null comparison on IFoo* must compile and execute correctly.
    if (p != nullptr)
    {
        outputBuffer[0] = 1;
    }
    else
    {
        // Null pointer: take the else branch.
        // CHECK: -1
        outputBuffer[0] = -1;
    }
}
