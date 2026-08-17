//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=Boundary (cross-target): the cuda-immutable-load pass fires only when emitting CUDA — HLSL emit of the same uniform read contains no `__ldg(...)` call.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The target-specific-lowering table lists `slang-ir-cuda-immutable-
// load.cpp` as gated on the CUDA target. The same Slang source emits
// `__ldg(&...)` on CUDA but no such intrinsic on HLSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// HLSL: void main
// HLSL-NOT: __ldg
// CUDA: __ldg
