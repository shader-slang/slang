// Passing a dynamically dispatched interface value to an unconstrained
// generic function triggers an internal compiler error. The compiler
// should either emit a clean diagnostic (like error 33180 for constrained
// generics) or support the pattern. Currently crashes with error 99999.
//
// Reproduces: https://github.com/shader-slang/slang/issues/10263

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain

interface IFoo
{
    float calc(float x);
}

struct A : IFoo
{
    float calc(float x) { return x * 2.0; }
}

struct B : IFoo
{
    float calc(float x) { return x + 10.0; }
}

T passthrough<T>(T x) { return x; }

IFoo makeObj(int id)
{
    if (id == 0) return A();
    else return B();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFoo obj = makeObj(id);

    // CHECK: error 99999
    IFoo result = passthrough(obj);

    outputBuffer[0] = result.calc(5.0);
}
