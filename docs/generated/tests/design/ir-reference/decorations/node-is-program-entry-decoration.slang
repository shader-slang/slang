//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#work-graph-nodes
//META: doc_section_digest=d3211cb2c8bf1946c6d21c3025a61938abd597a5f02eff70e030d767da85143a
//META: purpose=A [NodeIsProgramEntry] attribute lowers to a nodeIsProgramEntry decoration with no operands on the node function.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `nodeIsProgramEntry` row's operand shape: the marker is
// nullary, so the dump prints it bare. The CHECK pins the decoration and
// then `func %main`, showing it is a property of the node function rather
// than of one of its parameters.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null

import experimental.workgraph;

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NodeIsProgramEntry]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tid.x;
}

// CHECK: [nodeIsProgramEntry]
// CHECK: [nameHint("main")]
// CHECK: func %main
