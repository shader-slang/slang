//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#work-graph-nodes
//META: doc_section_digest=d3211cb2c8bf1946c6d21c3025a61938abd597a5f02eff70e030d767da85143a
//META: purpose=A [NodeMaxDispatchGrid(x,y,z)] attribute lowers to a nodeMaxDispatchGrid decoration carrying three integer operands, distinct from nodeDispatchGrid.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the upper-bound form of the dispatch grid is a separate
// opcode from the fixed form: the same three-integer operand shape appears
// under `nodeMaxDispatchGrid`. A node that declares a maximum grid must
// take its extents from an input record carrying `SV_DispatchGrid`, which
// is why the record type is used here rather than a plain thread id.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null

import experimental.workgraph;

struct GridData { uint3 grid : SV_DispatchGrid; };

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(16, 2, 1)]
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<GridData> input)
{
    outbuf[0] = input.Get().grid.x;
}

// CHECK: [nodeMaxDispatchGrid(16 : Int, 2 : Int, 1 : Int)]
// CHECK: [nameHint("main")]
// CHECK: func %main
