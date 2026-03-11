// Interface-typed entry point parameters require type conformance information
// to generate dispatch code. Without any registered conformances, the compiler
// cannot determine which concrete types may be passed and must reject the code.
//
// Disabled due to an assertion failure in IRUse::set() on x86_64 (#10266).
// Re-enable when that bug is fixed.

//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target cpp -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

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

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
// CHECK: ([[# @LINE+1]]): error 50100
void computeMain(uniform IFoo foo)
{
    outputBuffer[0] = foo.getValue();
}
