//TEST:COMPARE_COMPUTE(targets=vk+cuda+cpu+llvm, filecheck-buffer=CHECK):

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;


interface IFoo
{
    int foo(int a);
}

struct MyImpl : IFoo
{
    int foo(int a) { return a; }
}

int test(IFoo foo, int idx)
{
    int val = 0;
    if (let a = foo as MyImpl)
    {
        val = a.foo(idx);
    }
    return (val);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    MyImpl impl;
    // CHECK: 0
    // CHECK: 1
    // CHECK: 2
    // CHECK: 3
    outputBuffer[dispatchThreadID.x] = test(impl, dispatchThreadID.x);
}
