// Explicitly specializing a constrained generic function with an
// interface type triggers an internal compiler error instead of the
// expected error 33180 that implicit inference produces.

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

float genericFunc<T : IFoo>(T obj, float x)
{
    return obj.calc(x);
}

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
    float result = genericFunc<IFoo>(obj, 3.0);

    outputBuffer[0] = result;
}
