// ParameterBlock<IFoo> should support dynamic dispatch when type
// conformances are registered. ParameterBlock uses different binding
// semantics than ConstantBuffer (descriptor sets on Vulkan, root
// signatures on DX12), but the dispatch mechanism should be the same.

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

ParameterBlock<IFoo> gFoo;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = gFoo.getValue();
}

// Verify dispatch code is generated with both conformance types.
// The order of ImplA/ImplB in the output depends on witness table
// unique IDs, which may vary across platforms, so use DAG checks.
// HLSL-DAG: ImplA_getValue
// HLSL-DAG: ImplB_getValue
// HLSL: s_dispatch_IFoo_getValue
// HLSL: switch(
// HLSL: void computeMain

// SPIRV: OpEntryPoint
// CUDA: void computeMain
// CPP: void computeMain
// MTL: void computeMain
// WGSL: fn computeMain
