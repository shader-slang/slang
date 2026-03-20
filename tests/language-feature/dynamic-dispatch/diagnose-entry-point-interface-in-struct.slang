// A struct containing an interface-typed field used as an entry point
// parameter currently crashes (SPIRV) or triggers an internal error (HLSL).
// This test is disabled until the compiler properly handles existential
// types nested inside struct-typed entry point parameters.

//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -conformance "ImplA:IFoo=0"
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0"

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct Params
{
    IFoo foo;
    float scale;
}

RWStructuredBuffer<float> outputBuffer;

// CHECK: computeMain
[numthreads(1, 1, 1)]
void computeMain(uniform Params p)
{
    outputBuffer[0] = p.foo.getValue() * p.scale;
}
