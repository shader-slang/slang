//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#work-graph-record-types
//META: doc_section_digest=7ce9495f8f8cf2b1b4adf2f417316b6d259dafe3e374c502b180caf6e4a76ab2
//META: purpose=Indexing a NodeOutputArray<T> parameter yields a NodeOutput(<T>) value, and allocating from it yields ThreadNodeOutputRecords(<T>).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Three of the output-side record opcodes compose in one chain: the node
// array type, the single-node handle it indexes to, and the per-thread
// allocation taken off that handle. A broadcasting node takes a
// NodeOutputArray<MyRecord>, indexes element 0 into a local, and
// allocates one record from it; the IR dump is FileChecked for
// NodeOutputArray on the Func operand, NodeOutput on the indexed local,
// and ThreadNodeOutputRecords on the allocation result, so each opcode is
// observed at the point the surface construct produces it.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null

import experimental.workgraph;

struct MyRecord { uint value; }

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(4, 1, 1)]
[numthreads(4, 1, 1)]
void main(
    DispatchNodeInputRecord<MyRecord> input,
    [MaxRecords(4)] NodeOutputArray<MyRecord> outputs)
{
    NodeOutput<MyRecord> o = outputs[0];
    if (o.IsValid())
    {
        ThreadNodeOutputRecords<MyRecord> rec = o.GetThreadNodeOutputRecords(1);
        rec.Get().value = input.Get().value;
        rec.OutputComplete();
    }
}

// IR: func %main{{.*}}NodeOutputArray(%MyRecord)
// IR: let %o{{.*}}: NodeOutput(%MyRecord)
// IR: let %rec{{.*}}: ThreadNodeOutputRecords(%MyRecord)
