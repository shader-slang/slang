//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#type-constraint
//META: doc_section_digest=f8f9d27a9d4d52fa1ccabfb6595d02e760b74f56359f1c7d4cbf06dbfc7db829
//META: purpose=A constrained generic function specialized to a concrete type emits monomorphized target code on every text-emit back-end.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Type constraint" example emits per target: `addTwo<T>`
// with `where T : IArithmetic` is called at int type, so specialization
// monomorphizes it and the emitted entry point performs an integer add that
// escapes into the output buffer. Each target directive checks the entry point
// and buffer store survive to codegen on that back-end.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

func addTwo<T>(T a, T b) -> T where T : IArithmetic
{
    return a + b;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[id.x] = addTwo(int(id.x), 4);
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
