// Bare `uniform IFoo` at global scope with conformances should support
// dynamic dispatch, just like ConstantBuffer<IFoo> (global-interface-param.slang).
//
// Bare `uniform IFoo` with conformances crashes due to a pre-existing
// bug in the typeflow pass (separate from the ConstantBuffer/ParameterBlock
// fix). Disabled until the bare interface path is fixed.

//DISABLED_TEST:SIMPLE(filecheck=SINGLE): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0"
//DISABLED_TEST:SIMPLE(filecheck=SINGLE): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0"

//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"

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

uniform IFoo gFoo;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = gFoo.getValue();
}

// SINGLE: computeMain
// CHECK: computeMain
