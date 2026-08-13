//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#generic-structure-with-type-and-value-parameters
//META: doc_section_digest=da081aceaa02998e40ea6cc22b6ac1d29064f4b32ae25b1a60fc6c17e56e7e73
//META: purpose=A specialized generic struct emits as a concrete monomorphized entry point on every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that binding the generic parameters of `TestStruct<T, let size>`
// specializes it into a concrete type before codegen (doc: "a specialized
// generic is a concrete type ... which can be used like any other concrete
// type"). Each target directive asserts the entry point emits and the buffer
// store is present in the target-specific text, proving monomorphization
// reached codegen on that back-end.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct TestStruct<T, let size : uint>
{
    T arr[size];
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(3, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    TestStruct<float, 3> obj = { 10, 20, 30 };
    outputBuffer[id.x] = obj.arr[id.x];
}

// HLSL: void computeMain
// HLSL: outputBuffer
// SPV: OpEntryPoint GLCompute
// SPV: %outputBuffer
// GLSL: void main(
// GLSL: outputBuffer
// METAL-DAG: computeMain
// METAL-DAG: outputBuffer
// WGSL: fn computeMain
// WGSL: outputBuffer
// CPP-DAG: computeMain
// CPP-DAG: outputBuffer
