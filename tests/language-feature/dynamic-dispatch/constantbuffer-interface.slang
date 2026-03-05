// ConstantBuffer<IFoo> should support dynamic dispatch when type
// conformances are registered, generating a switch-based dispatch for
// method calls on the interface-typed constant buffer element.
//
// Currently crashes with a null-pointer dereference (SEGV) when
// conformances are provided via -conformance flags. The same crash
// affects ParameterBlock<IFoo>. Without conformances, the compiler
// correctly emits error 50100. Disabled until #10314 is fixed.

//DISABLED_TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"

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
    outputBuffer[0] = gFoo.getValue();
}

// SPIRV: OpEntryPoint
// HLSL: void computeMain
// CUDA: void computeMain
// CPP: void computeMain
// MTL: void computeMain
// WGSL: fn computeMain
