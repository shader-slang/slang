//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#work-graph-record-types
//META: doc_section_digest=301cf5fbdc053cefff914c60b0a6b2763a73ba2f02169943b38c045d43b6e5fb
//META: purpose=A ThreadNodeInputRecord<T> node parameter is typed ThreadNodeInputRecord(<T>) in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The per-thread input record of a thread-launch node has its own type
// opcode, distinct from the broadcasting node's dispatch record even
// though both wrap the same record struct. A thread-launch entry point
// takes ThreadNodeInputRecord<RecordData>; the IR dump is FileChecked at
// `func %main` for ThreadNodeInputRecord(%RecordData), which shows the
// launch mode selects the opcode, not just a decoration.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o -

import experimental.workgraph;

struct RecordData { int value; }

RWStructuredBuffer<int> outputBuffer;

[shader("node")]
[NodeLaunch("thread")]
void main(ThreadNodeInputRecord<RecordData> input)
{
    outputBuffer[0] = input.Get().value;
}

// IR: func %main{{.*}}: Func(Void, ThreadNodeInputRecord(%RecordData)
// IR: param %input{{.*}}: ThreadNodeInputRecord(%RecordData)
