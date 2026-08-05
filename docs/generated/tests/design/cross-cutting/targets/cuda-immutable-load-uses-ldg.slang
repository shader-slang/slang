//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=fd65eacd7dd52604642d568dd04e37acc830eba9b906dbcfc4a4d6f3d0cf9a61
//META: purpose=The CUDA-specific immutable-load lowering pass rewrites reads from `uniform` globals as `__ldg(...)` calls on CUDA only, while HLSL, GLSL, SPIR-V, Metal, WGSL and C++ read the field directly.
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
