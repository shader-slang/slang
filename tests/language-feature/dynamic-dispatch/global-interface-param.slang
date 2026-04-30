// Global shader parameters of interface type should support dynamic dispatch
// when type conformances are registered via -conformance flags. The compiler
// should generate dispatch code for method calls on the global parameter,
// just as it does for entry-point interface parameters.
//
// Two complementary verifications:
//
// (1) REPORT — `-report-dynamic-dispatch-sites` (E50102) is the
//     stable structural contract that the compiler actually
//     generated dispatch code for the multi-conformance call site.
//     Target-independent and survives renames of synthesized
//     dispatch shims.
//
// (2) Cross-target compiles confirm each backend produces an entry
//     point without crashing on `ConstantBuffer<IFoo>`. End-to-end
//     runtime correctness for the IRPointerLikeType
//     (ConstantBuffer/ParameterBlock) fix path is covered by the
//     `global-interface-param-compute.slang` and
//     `constantbuffer-struct-interface-field.slang` compute tests,
//     which exercise the same `specializeLoad` /
//     `lowerExtractDynamicObject` paths.
//
// (3) SINGLE — single-conformance variant pins the monomorphization
//     contract via `-NOT` checks: with only one impl registered, no
//     dispatch shim should be emitted. Verifies `seedGlobalScope` /
//     `TaggedUnionType` handle the singleton set correctly.

//TEST:SIMPLE(filecheck=REPORT): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -report-dynamic-dispatch-sites
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=SINGLE): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -report-dynamic-dispatch-sites

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
    // REPORT-DAG: note[E50102]: generated{{.*}}dispatch code{{.*}}2 possible types: 'ImplA, ImplB'
    // REPORT-DAG: :[[# @LINE+2]]:
    // SINGLE-NOT: E50102
    outputBuffer[0] = gFoo.getValue();
}

// Each backend must produce the entry point without crashing.
// SPIRV: OpEntryPoint
// HLSL: void computeMain
// GLSL: void main()
// CUDA: void computeMain
// MTL: void computeMain
// WGSL: fn computeMain
// CPP: void computeMain
