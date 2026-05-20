//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#emit-dispatcher
//META: doc_section_digest=05e360846fc3ee11f7582f6896e802ffbb7981b023d417a3b600718f4844ea5b
//META: purpose=emitEntryPointsSourceFromIR dispatches to the per-target backend for HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Compiles the same source to every text-emit target supported in the
// no-GPU runner. Each //TEST directive FileChecks for a marker that
// only the corresponding backend would emit. If the dispatcher were
// missing a registration for any backend, the corresponding sub-test
// would fail with no output.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HLSL: numthreads
// GLSL: #version
// SPIRV: OpEntryPoint GLCompute
// METAL: metal_stdlib
// WGSL: @compute
// CUDA: __global__
// CPP: SLANG_PRELUDE_EXPORT
