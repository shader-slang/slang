// Global shader parameters of interface type require type conformance
// information to generate dispatch code. Without registered conformances,
// the compiler cannot determine which concrete types may be stored in
// the parameter and must reject the code with error 50100.
//
// The ConstantBuffer<IFoo> wrapper is the supported way to declare
// interface-typed globals for dynamic dispatch (bare `uniform IFoo`
// at global scope crashes; see global-interface-param-bare.slang).

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target cpp -stage compute -entry computeMain

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

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2.0; }
}

ConstantBuffer<IFoo> gFoo;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: error 50100
    outputBuffer[0] = gFoo.getValue();
}
