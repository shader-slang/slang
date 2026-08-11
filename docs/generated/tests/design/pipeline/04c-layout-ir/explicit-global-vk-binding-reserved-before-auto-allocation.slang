//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#parameter-binding-producing-the-programlayout
//META: doc_section_digest=7da97e6272e7230be6aa860fcb219926e5ecf2fe260c372906e4be02b8c61abb
//META: purpose=Explicit vk::binding annotations on global parameters are reserved before any automatic allocation, so an earlier unannotated global skips the reserved slot.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ordering of the binding stages. `autoBuf` is declared first
// and carries no annotation, while `pinnedBuf` is pinned to binding 0. If
// automatic allocation ran first, `autoBuf` would take binding 0 and
// collide; CHECK asserts it is placed at binding 1 on all three gate-honoring
// targets, so the explicit reservation was recorded first.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> autoBuf;
[[vk::binding(0, 0)]] RWStructuredBuffer<int> pinnedBuf;

[numthreads(1, 1, 1)]
void main()
{
    autoBuf[0] = pinnedBuf[0];
}

// SPV-DAG: OpDecorate %autoBuf{{[A-Za-z0-9_]*}} Binding 1
// SPV-DAG: OpDecorate %pinnedBuf{{[A-Za-z0-9_]*}} Binding 0

// GLSL-DAG: binding = 1) buffer
// GLSL-DAG: binding = 0) buffer

// WGSL-DAG: @binding(1) @group(0) var<storage, read_write> autoBuf
// WGSL-DAG: @binding(0) @group(0) var<storage, read_write> pinnedBuf
