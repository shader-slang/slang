//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#emitting-hlsl-named-constants-rather-than-integers
//META: doc_section_digest=788bc7519655c8e116120260bbcac10f921c9993b6dbc20665b3dec1d2d85f1b
//META: purpose=Work-graph record types emit under their HLSL type names with an `<ElementType>` argument — `DispatchNodeInputRecord<T>`, `NodeOutput<T>`, `ThreadNodeOutputRecords<T>` — recovered from the opcode-to-name table.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that three different work-graph record-type opcodes each resolve
// to their own HLSL type spelling rather than to a shared or numeric form.
// The entry point takes a `DispatchNodeInputRecord<T>` input and a
// `NodeOutput<T>` output, and allocates a `ThreadNodeOutputRecords<T>` from
// the output; CHECK asserts all three names reach the emitted text carrying
// the user struct as their element-type argument. Because the element type
// is mangled on emit, the CHECKs wildcard the suffix but pin the record type
// names, which is the part the opcode-to-name table decides.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

import experimental.workgraph;

struct NodeRecord
{
    uint value;
}

RWStructuredBuffer<uint> outBuf;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(
    DispatchNodeInputRecord<NodeRecord> inRec,
    [MaxRecords(4)] NodeOutput<NodeRecord> outRec)
{
    outBuf[0] = inRec.Get().value;
    ThreadNodeOutputRecords<NodeRecord> recs = outRec.GetThreadNodeOutputRecords(1);
    recs.OutputComplete();
}

// The input-record and output-node parameter types keep their HLSL names.
// CHECK-DAG: DispatchNodeInputRecord<NodeRecord_{{[0-9]+}}>
// CHECK-DAG: NodeOutput<NodeRecord_{{[0-9]+}}>
// The allocated output-records local resolves to its own distinct name.
// CHECK-DAG: ThreadNodeOutputRecords<NodeRecord_{{[0-9]+}}>
