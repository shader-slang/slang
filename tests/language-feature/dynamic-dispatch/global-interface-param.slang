// Global shader parameters of interface type should support dynamic dispatch
// when type conformances are registered via -conformance flags. The compiler
// should generate dispatch code for method calls on the global parameter,
// just as it does for entry-point interface parameters.
//
// Currently all targets crash (segfault) when ConstantBuffer<IFoo> globals
// are used with -conformance flags, and bare `uniform IFoo` globals crash
// with ICE 99999 ("Unexpected context type for parameter info retrieval").
// Disabled until these bugs are fixed (#10314).

//DISABLED_TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=REPORT): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -report-dynamic-dispatch-sites

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
    // REPORT: ([[# @LINE+1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[0] = gFoo.getValue();
}

// SPIRV: OpEntryPoint
// HLSL: void computeMain
// CUDA: void computeMain
