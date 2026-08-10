//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#nodelaunch-and-the-work-graph-node-decorations
//META: doc_section_digest=daa05cb203fdd0b8d97d568a362266b28c42f4928418925387113d90d02e73f3
//META: purpose=nodeArraySize and allowSparseNodes land on an output-node-array parameter rather than on the node function, alongside a parameter-level maxRecords.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the position claim that separates this group from the other
// node attributes: `nodeArraySize` and `allowSparseNodes` decorate the
// output-array *parameter*, while `nodeLaunch` and friends decorate the
// function. The CHECK walks in order - function-level `nodeLaunch`, then
// `func %main`, then the three parameter-level decorations, then
// `param %outputs` - so a lowering that hoisted the parameter attributes
// onto the function would break the ordering and fail.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null

import experimental.workgraph;

struct OutputRecord { uint value; };

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(
    [MaxRecords(4)]
    [NodeArraySize(8)]
    [AllowSparseNodes]
    NodeOutputArray<OutputRecord> outputs)
{
    outbuf[0] = 1u;
}

// CHECK: [nodeLaunch("broadcasting")]
// CHECK: func %main
// CHECK: [allowSparseNodes]
// CHECK: [nodeArraySize(8 : Int)]
// CHECK: [maxRecords(4 : Int)]
// CHECK: param %outputs
