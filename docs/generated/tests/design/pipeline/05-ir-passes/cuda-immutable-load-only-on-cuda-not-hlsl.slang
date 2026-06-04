//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
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
