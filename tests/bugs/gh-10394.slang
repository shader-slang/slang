// Test that passing a struct with an interface-typed field by value to a
// function compiles successfully for the CPU/C++ target.
// See: https://github.com/shader-slang/slang/issues/10394

//TEST:SIMPLE(filecheck=CHECK): -target cpp -stage compute -entry computeMain -conformance "ImplA:IFoo=0"

[anyValueSize(8)]
interface IFoo
{
    int getValue();
}

struct ImplA : IFoo
{
    int v;
    int getValue() { return v; }
}

struct Params
{
    IFoo foo;
    int scale;
}

int dispatch(Params p)
{
    return p.foo.getValue() * p.scale;
}

ConstantBuffer<Params> gParams;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = dispatch(gParams);
}

// CHECK: getValue
// CHECK: computeMain
