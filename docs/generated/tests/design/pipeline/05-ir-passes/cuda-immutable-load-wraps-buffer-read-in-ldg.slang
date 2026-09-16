//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-16T06:40:19Z
//META: source_commit=d3b56927b854c14fa1ac169b72da6e080ea80833
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=The CUDA immutable-load pass wraps read-only `StructuredBuffer` element loads in `__ldg(&...)` on the CUDA target.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "CUDA immutable load" row of the target-specific lowering
// table (`slang-ir-cuda-immutable-load.cpp`) rewrites eligible
// global-memory loads from a read-only buffer into `__ldg(...)`
// intrinsic reads on the CUDA backend.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

StructuredBuffer<int> input;
RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = input[tid.x] + 1;
}

// CUDA: __ldg(
