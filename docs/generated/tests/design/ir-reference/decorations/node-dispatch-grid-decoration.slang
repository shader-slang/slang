//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#work-graph-nodes
//META: doc_section_digest=5226e00be7c0cb4031cddbe6e885685396e42c7841a4bb73bc9f271385a6ae1c
//META: purpose=A [NodeDispatchGrid(x,y,z)] attribute lowers to a nodeDispatchGrid decoration carrying three integer operands in source order.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand count and ordering of `nodeDispatchGrid`, the fixed
// dispatch-grid size of a broadcasting node. Three distinct extents (4, 2,
// 1) are used so a transposed lowering cannot accidentally match; the CHECK
// pins the exact triple and then the `func %main` the decoration sits on.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o -

import experimental.workgraph;

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(4, 2, 1)]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tid.x;
}

// CHECK: [nodeDispatchGrid(4 : Int, 2 : Int, 1 : Int)]
// CHECK: [nameHint("main")]
// CHECK: func %main
