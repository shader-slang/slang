//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#wgsl
//META: doc_section_digest=24cf127ba87a077887a4ad8e725f5ae07b1714ceed5cc4f322ad01d02f3bfd6e
//META: purpose=The WGSL backend emits WGSL source with `@binding(N) @group(N)` resource attributes and `@compute` / `@workgroup_size(...)` entry-point attributes.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the WGSL backend produces WGSL rather than C-like text. CHECK
// asserts the WGSL-only attribute vocabulary: the storage-buffer variable is
// declared with `var<storage, read_write>` behind `@binding`/`@group`, and the
// entry point is annotated `@compute` with an explicit `@workgroup_size`.
// A `fn ... -> ` free function is also asserted so a regression that emitted
// C-style declarations would be caught.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int doubleIt(int v)
{
    return v * 2;
}

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = doubleIt(int(tid.x));
}

// WGSL-DAG: @binding(0) @group(0) var<storage, read_write> outBuf_{{[0-9]+}} : array<i32>
// WGSL-DAG: {{^}}@compute
// WGSL-DAG: {{^}}@workgroup_size(8, 1, 1)
// WGSL-DAG: fn doubleIt_{{[0-9]+}}( v_{{[0-9]+}} : i32) -> i32
