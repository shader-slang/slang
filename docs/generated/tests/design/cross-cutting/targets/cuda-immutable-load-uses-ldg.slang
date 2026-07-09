//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=6f7f7ef77b73b73b7a036bdae99ff624ab46818ddb538ebde1c189c05207c5c0
//META: purpose=The CUDA target-specific lowering pass (`slang-ir-cuda-immutable-load.cpp`, named in the doc) rewrites reads from `uniform` globals as `__ldg(...)` calls on CUDA only; HLSL/GLSL/SPIR-V/Metal/WGSL/CPP read the field directly.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc's "How target choice affects IR" list, CUDA carries a
// dedicated pass `slang-ir-cuda-immutable-load.cpp`. Its observable
// effect is that a load from a `uniform` (read-only) global lowers
// to a `__ldg(&...)` intrinsic on CUDA. The same load on every
// other text-emit target is a plain field reference.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform int gValue;
uniform RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = gValue;
}

// CUDA: __ldg(
// HLSL-NOT: __ldg
// CPP-NOT: __ldg
