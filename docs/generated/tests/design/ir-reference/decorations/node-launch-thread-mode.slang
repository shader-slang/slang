//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#nodelaunch-and-the-work-graph-node-decorations
//META: doc_section_digest=daa05cb203fdd0b8d97d568a362266b28c42f4928418925387113d90d02e73f3
//META: purpose=A [NodeLaunch("thread")] attribute lowers to a nodeLaunch decoration whose mode operand is the quoted string "thread", not an integer tag.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the claim that the launch mode reaches the IR as a named
// constant: the operand prints as a quoted string, so no numeric mapping
// sits between the source attribute and the backend. The CHECK pins
// `[nodeLaunch("thread")]` and then the `func %main` it decorates. A
// thread-launch node may not carry `[numthreads]`, so none is written.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null

import experimental.workgraph;

struct MyRecord { uint value; };

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("thread")]
void main(ThreadNodeInputRecord<MyRecord> input)
{
    outbuf[0] = input.Get().value;
}

// CHECK: [nodeLaunch("thread")]
// CHECK: [nameHint("main")]
// CHECK: func %main
