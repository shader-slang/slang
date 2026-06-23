//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
//META: purpose=The Metal-legalize pass attaches positional `[[buffer(N)]]` annotations and `kernel` markers to the entry point.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Metal legalize" entry in the target-specific lowering table
// (`slang-ir-metal-legalize.cpp`) rewrites entry-point parameters
// to use Metal's positional `[[buffer(N)]]` binding model with a
// `kernel` qualifier. Per `_common.md`, `[[...]]` is FileCheck
// variable syntax; match the bare token instead.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 7;
}

// METAL: kernel
// METAL-SAME: thread_position_in_grid
// METAL-SAME: buffer(0)
