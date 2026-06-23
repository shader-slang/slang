//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
//META: purpose=The WGSL-legalize pass attaches `@binding(N) @group(N)` to resources and `@compute @workgroup_size(...)` to the entry point.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "WGSL legalize" entry in the target-specific lowering table
// (`slang-ir-wgsl-legalize.cpp`) rewrites resources and entry-point
// markers into WGSL-shaped attributes.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(2, 3, 4)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 7;
}

// WGSL: @binding({{[0-9]+}}) @group({{[0-9]+}})
// WGSL: @compute
// WGSL: @workgroup_size(2, 3, 4)
