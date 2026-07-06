//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#examples
//META: doc_section_digest=2843e3e730992c3b4a58e5dcb338c66a5db5be57ab33d9c88e766fe31406de61
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
