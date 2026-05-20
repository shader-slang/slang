//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
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
