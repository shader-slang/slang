//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#hlsl
//META: doc_section_digest=aa6de29bd5d66aaa6fc85d4bb699f7a68e29ec9dc1dc52c392fa478b055ef466
//META: purpose=Work-graph entry-point attributes are re-emitted from their IR decorations rather than from integers — `[NodeLaunch("...")]` keeps its string payload, and `[NodeDispatchGrid]`, `[NodeID]`, `[NodeIsProgramEntry]` plus the parameter-level `[MaxRecords]`, `[NodeArraySize]`, `[AllowSparseNodes]` are all spelled by name.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the symbolic round-trip of work-graph attributes through the HLSL
// backend. The launch mode is carried as a string literal in the IR
// decoration, so the emitted attribute must reproduce the same quoted text
// rather than the enum ordinal DXC would not accept. CHECK asserts each
// entry-point attribute on the emitted function and each parameter attribute
// on the emitted parameter list.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

import experimental.workgraph;

struct OutputRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NodeID("myNode", 3)]
[NodeIsProgramEntry]
[numthreads(1, 1, 1)]
void main(
    RWBuffer<int> buf,
    [MaxRecords(4)] [NodeArraySize(2)] [AllowSparseNodes] NodeOutputArray<OutputRecord> outputs)
{
    buf[0] = outputs[0].IsValid() ? 1 : 0;
}

// HLSL-DAG: [NodeLaunch("broadcasting")]
// HLSL-DAG: {{^}}[NodeDispatchGrid(2, 1, 1)]
// HLSL-DAG: {{^}}[NodeID("myNode", 3)]
// HLSL-DAG: {{^}}[NodeIsProgramEntry]
// HLSL-DAG: [MaxRecords(4)] [NodeArraySize(2)] [AllowSparseNodes] NodeOutputArray<
