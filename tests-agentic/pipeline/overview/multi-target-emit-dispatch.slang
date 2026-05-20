//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/overview.md#emit
//META: doc_section_digest=d7c5155aced5a19f1303c1d313c7cf3b21da58b38cefd801d92e52039ee02528
//META: purpose=The emit dispatcher selects a backend per TargetRequest and produces target text for HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the overview claim that emitEntryPointsSourceFromIR routes a
// compile to a per-target backend. The same trivial shader is compiled
// to each of the seven text-emit backends supported in the no-GPU
// runner; each //TEST directive FileChecks for a target-distinctive
// marker that only that backend would emit. If the dispatcher were
// missing a path for any backend, the corresponding sub-test would
// fail.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // HLSL: numthreads
    // GLSL: local_size_x
    // SPIRV: OpEntryPoint
    // METAL: kernel
    // WGSL: @compute
    // CUDA: __global__
    // CPP: SLANG_PRELUDE_NAMESPACE
    return;
}
