// Test interface qualifier behavior under `-lang slang -std 2026`.
//
// Verifies that `dyn IFoo` and unqualified `IFoo` parameters compile
// and dispatch correctly under 2026 mode. The implicit `some` semantics
// for unqualified interfaces are not yet implemented (SP#024, tracked by #7115).

//TEST:SIMPLE(filecheck=CHECK): -target spirv -lang slang -std 2026 -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -lang slang -std 2026 -stage compute -entry computeMain

interface IFoo
{
    int get();
}

struct A : IFoo
{
    int val;
    int get() { return val; }
}

struct B : IFoo
{
    int val;
    int get() { return val * 2; }
}

// Unqualified `IFoo` parameter: currently specialized. Once `some` is
// implemented, this should enforce `some` semantics under 2026 mode.
int callFoo(IFoo x)
{
    return x.get();
}

int callFooDyn(dyn IFoo x)
{
    return x.get();
}

// CHECK: OpEntryPoint GLCompute %computeMain
// CHECK: OpAccessChain
// CHECK: OpStore

// CHECK_HLSL: void computeMain

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    A a = {3};
    B b = {5};

    outputBuffer[0] = callFoo(a);
    outputBuffer[1] = callFooDyn(b);
}
