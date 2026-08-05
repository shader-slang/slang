//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#nodelaunch-and-the-work-graph-node-decorations
//META: doc_section_digest=daa05cb203fdd0b8d97d568a362266b28c42f4928418925387113d90d02e73f3
//META: purpose=A [NodeLaunch("coalescing")] attribute lowers to nodeLaunch("coalescing"), and a [MaxRecords(N)] on the input-record parameter lowers to maxRecords on that parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third launch-mode spelling keeps its own string operand, and
// that `maxRecords` can sit on a parameter rather than on the node function
// - the doc says that attribute is accepted in either position. The CHECK
// sequence walks from the function-level `nodeLaunch` through `func %main`
// into the parameter list, where the parameter-level `maxRecords` must
// appear.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null

import experimental.workgraph;

struct MyRecord { uint value; };

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("coalescing")]
[numthreads(4, 1, 1)]
void main([MaxRecords(8)] GroupNodeInputRecords<MyRecord> inputs)
{
    outbuf[0] = inputs.Get(0).value;
}

// CHECK: [nodeLaunch("coalescing")]
// CHECK: func %main
// CHECK: [maxRecords(8 : Int)]
// CHECK: param %inputs
