//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-16T06:40:19Z
//META: source_commit=d3b56927b854c14fa1ac169b72da6e080ea80833
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=Boundary (cross-target): the CUDA immutable-load pass wraps a read-only buffer load in `__ldg(...)`, while HLSL emission of the same load has no CUDA intrinsic.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The target-specific-lowering table lists `slang-ir-cuda-immutable-
// load.cpp` as gated on the CUDA target. The same read-only buffer
// source emits `__ldg(&...)` on CUDA but no such intrinsic on HLSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

StructuredBuffer<int> input;
RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = input[tid.x] + 1;
}

// HLSL-NOT: __ldg
// HLSL: input_{{[0-9]+}}.Load(
// HLSL-NOT: __ldg
// CUDA: __ldg
