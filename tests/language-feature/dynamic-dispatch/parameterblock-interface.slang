// ParameterBlock<IFoo> should support dynamic dispatch when type
// conformances are registered. ParameterBlock uses different binding
// semantics than ConstantBuffer (descriptor sets on Vulkan, root
// signatures on DX12), but the dispatch mechanism should be the same.
//
// Two complementary verifications:
//
// (1) REPORT — `-report-dynamic-dispatch-sites` is the stable
//     structural contract that the compiler generated dispatch code
//     for the multi-conformance call site. Pinning the diagnostic
//     (E50102) is target-independent and survives renames of the
//     synthesized dispatch shim.
//
// (2) Cross-target compiles confirm each backend produces an entry
//     point without crashing on `ParameterBlock<IFoo>`. End-to-end
//     runtime correctness for the IRPointerLikeType (ConstantBuffer/
//     ParameterBlock) fix path is covered by the
//     `parameterblock-struct-interface-field.slang` and
//     `global-interface-param-compute.slang` compute tests, which
//     exercise the same `specializeLoad` / `lowerExtractDynamicObject`
//     paths fixed by this PR.

//TEST:SIMPLE(filecheck=REPORT): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -report-dynamic-dispatch-sites
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
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
    // The type list ('ImplA, ImplB' or 'ImplB, ImplA') is iteration-
    // order dependent across platforms — pin only the count + line.
    // REPORT-DAG: note[E50102]: generated{{.*}}dispatch code{{.*}}2 possible types:
    // REPORT-DAG: :[[# @LINE+1]]:
    outputBuffer[0] = gFoo.getValue();
}

// Each backend must produce the entry point without crashing.
// SPIRV: OpEntryPoint
// HLSL: void computeMain
// GLSL: void main
// CUDA: void computeMain
// CPP: void computeMain
// MTL: void computeMain
// WGSL: fn computeMain
