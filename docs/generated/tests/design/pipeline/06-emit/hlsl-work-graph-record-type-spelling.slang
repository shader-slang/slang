//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#hlsl
//META: doc_section_digest=aa6de29bd5d66aaa6fc85d4bb699f7a68e29ec9dc1dc52c392fa478b055ef466
//META: purpose=Work-graph record types are spelled by mapping the IR opcode to its HLSL type name and re-emitting the element type as a generic argument, so `DispatchNodeInputRecord<T>`, `NodeOutput<T>`, and `GroupNodeOutputRecords<T>` all come out named with their element struct.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the record-type spelling path in the HLSL backend. These types
// exist as distinct IR opcodes rather than ordinary generic structs, so the
// emitter has to reconstruct both the HLSL name and the `<ElementType>`
// argument. Three different record kinds are used in one node shader — an
// input record, an output handle, and an allocated output-record group — and
// CHECK asserts each is emitted with the user's own record struct as its
// generic argument.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

import experimental.workgraph;

struct InRecord
{
    int value;
};

struct OutRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(
    DispatchNodeInputRecord<InRecord> input,
    RWBuffer<int> buf,
    [MaxRecords(2)] NodeOutput<OutRecord> output)
{
    GroupNodeOutputRecords<OutRecord> recs = output.GetGroupNodeOutputRecords(1);
    buf[0] = input.Get().value;
    recs.OutputComplete();
}

// HLSL-DAG: DispatchNodeInputRecord<InRecord_{{[0-9]+}}> input_{{[0-9]+}}
// HLSL-DAG: NodeOutput<OutRecord_{{[0-9]+}}> output_{{[0-9]+}}
// HLSL-DAG: GroupNodeOutputRecords<OutRecord_{{[0-9]+}}> recs_{{[0-9]+}}
