//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=6399b7c2a0c9dbf90340a8eb7cb86baa3cd9c07289fb6d93436a49b19618df81
//META: purpose=A work-graph node input record parameter uses the `DispatchNodeInputRecord` type opcode carrying its element type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `WorkGraphRecordTypeBase` type row, which lists
// `DispatchNodeInputRecord` / `NodeOutput` / `EmptyNodeOutput` as a
// subfamily whose members carry a single `elementType` operand. A node
// entry point that takes `DispatchNodeInputRecord<MyRecord>` is the source
// surface; the IR CHECK asserts the parameter's type prints as
// `DispatchNodeInputRecord(%MyRecord)` — the opcode applied to the record
// struct — which is only true if the element type is an operand of the type
// inst rather than baked into a distinct opcode per record struct.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage node -entry myNode -profile lib_6_8 -experimental-feature -dump-ir -o -

import experimental.workgraph;

struct MyRecord { uint value; };

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void myNode(DispatchNodeInputRecord<MyRecord> input)
{
    outbuf[0] = input.Get().value;
}

// IR-LABEL: func %myNode
// IR: param %input : DispatchNodeInputRecord(%MyRecord)
