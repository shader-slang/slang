// Verify that StructuredBuffer<IFoo> and RWStructuredBuffer<IFoo> compile
// successfully across all target backends.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=1" -conformance "ImplB:IFoo=2"
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=1" -conformance "ImplB:IFoo=2"
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry computeMain -conformance "ImplA:IFoo=1" -conformance "ImplB:IFoo=2"
//TEST:SIMPLE(filecheck=MTL):-target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=1" -conformance "ImplB:IFoo=2"
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry computeMain -conformance "ImplA:IFoo=1" -conformance "ImplB:IFoo=2"

[anyValueSize(8)]
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

StructuredBuffer<IFoo> inputBuffer;
RWStructuredBuffer<IFoo> rwBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo readVal = inputBuffer[0];
    outputBuffer[0] = readVal.getValue();

    IFoo rwVal = rwBuffer[0];
    outputBuffer[1] = rwVal.getValue();

    ImplA a;
    a.v = 1.0;
    rwBuffer[0] = a;
}

// SPIRV: OpEntryPoint GLCompute
// HLSL: void computeMain
// CUDA: void computeMain
// MTL: void computeMain
// WGSL: fn computeMain
