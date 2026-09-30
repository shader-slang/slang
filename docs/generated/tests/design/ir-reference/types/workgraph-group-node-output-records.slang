//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#work-graph-record-types
//META: doc_section_digest=301cf5fbdc053cefff914c60b0a6b2763a73ba2f02169943b38c045d43b6e5fb
//META: purpose=Allocating group-wide output records off a NodeOutput<T> yields a value typed GroupNodeOutputRecords(<T>).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The per-group output allocation is a separate opcode from the
// per-thread one even though both come from the same NodeOutput handle.
// A broadcasting node takes a bare NodeOutput<MyRecord> parameter and
// calls GetGroupNodeOutputRecords; the IR dump is FileChecked for the
// NodeOutput operand on `func %main` and for GroupNodeOutputRecords on
// the local holding the allocation, which distinguishes the group-side
// opcode from the thread-side one exercised by the node-output-array test.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o -

import experimental.workgraph;

struct MyRecord { uint value; }

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(4, 1, 1)]
void main(
    DispatchNodeInputRecord<MyRecord> input,
    [MaxRecords(1)] NodeOutput<MyRecord> output)
{
    GroupNodeOutputRecords<MyRecord> rec = output.GetGroupNodeOutputRecords(1);
    rec.Get().value = input.Get().value;
    rec.OutputComplete();
}

// IR: func %main{{.*}}NodeOutput(%MyRecord)
// IR: let %rec{{.*}}: GroupNodeOutputRecords(%MyRecord)
