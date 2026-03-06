// Verify that ConstantBuffer and ParameterBlock wrapping a struct
// with interface-typed fields compile successfully across all code
// generation targets. This covers the full matrix of resource wrapper
// types (ConstantBuffer, ParameterBlock) x field patterns (single
// interface field, interface array field) x all backend targets.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"

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

struct SingleFieldParams
{
    IFoo foo;
    float scale;
}

struct ArrayFieldParams
{
    IFoo values[2];
}

ConstantBuffer<SingleFieldParams> gCbSingle;
ConstantBuffer<ArrayFieldParams> gCbArray;
ParameterBlock<SingleFieldParams> gPbSingle;
ParameterBlock<ArrayFieldParams> gPbArray;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float result = 0.0;
    result += gCbSingle.foo.getValue() * gCbSingle.scale;
    result += gCbArray.values[0].getValue() + gCbArray.values[1].getValue();
    result += gPbSingle.foo.getValue() * gPbSingle.scale;
    result += gPbArray.values[0].getValue() + gPbArray.values[1].getValue();
    outputBuffer[0] = result;
}

// SPIRV: OpEntryPoint GLCompute %computeMain
// HLSL: void computeMain
// CUDA: void computeMain
// CPP: void _computeMain
// MTL: void computeMain
// WGSL: fn computeMain
