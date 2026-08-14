//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#work-graph-record-types
//META: doc_section_digest=7ce9495f8f8cf2b1b4adf2f417316b6d259dafe3e374c502b180caf6e4a76ab2
//META: purpose=A DispatchNodeInputRecord<T> node parameter is typed DispatchNodeInputRecord(<T>) in the IR dump, with the record struct as its single operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A broadcasting-launch node's read-only input record lowers to the
// DispatchNodeInputRecord type opcode whose single operand is the record
// struct T. A node entry point takes DispatchNodeInputRecord<MyRecord>
// and the IR dump is FileChecked at `func %main` for the opcode carrying
// `%MyRecord`; the parameter row inside the block confirms the same type
// reaches the parameter, which is only true if lowering materialised the
// opcode rather than an ordinary DeclRefType-shaped struct.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null

import experimental.workgraph;

struct MyRecord { uint value; }

RWStructuredBuffer<uint> outputBuffer;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<MyRecord> input)
{
    outputBuffer[0] = input.Get().value;
}

// IR: func %main{{.*}}: Func(Void, DispatchNodeInputRecord(%MyRecord)
// IR: param %input{{.*}}: DispatchNodeInputRecord(%MyRecord)
