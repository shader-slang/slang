//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=The CUDA immutable-load pass wraps reads of uniform globals in `__ldg(&...)` on the CUDA target.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "CUDA immutable load" row of the target-specific lowering
// table (`slang-ir-cuda-immutable-load.cpp`) rewrites loads of
// uniform globals into `__ldg(...)` intrinsic reads on the CUDA
// backend. Per `_common.md`, CUDA factors compound expressions over
// `__ldg` reads into temporaries — we observe the single `__ldg(&...)`
// token only.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// CUDA: __ldg(&
