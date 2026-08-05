//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#emitting-hlsl-named-constants-rather-than-integers
//META: doc_section_digest=d4f75e0916ecd748a0aa9042c7a027c3f7560681b96694b8a6f4e2edbc24668f
//META: purpose=A node entry point's launch mode is emitted as the verbatim string `[NodeLaunch("broadcasting")]`, while `[NodeMaxDispatchGrid(x, y, z)]`, whose operands really are integers, keeps its numeric spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the split the HLSL backend draws between attribute operands that
// are *names* and operands that are genuinely *numbers*. The launch mode is
// carried through the pipeline as a string literal and copied out verbatim,
// so CHECK asserts `[NodeLaunch("broadcasting")]` appears with the quoted
// mode intact; the dispatch-grid bounds are real integers, so the following
// CHECK asserts `[NodeMaxDispatchGrid(4, 2, 1)]` keeps its numeric operands.
// Seeing the quoted mode next to the numeric grid in one emit proves the
// choice is per-attribute, not a blanket stringification.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

import experimental.workgraph;

struct NodeRecord
{
    uint value;
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(4, 2, 1)]
[numthreads(2, 1, 1)]
void main(DispatchNodeInputRecord<NodeRecord> inRec)
{
}

// The stage name and the launch mode are both quoted strings copied verbatim.
// CHECK: [shader("node")]
// CHECK-SAME: [NodeLaunch("broadcasting")]
// The dispatch-grid operands are genuine integers and stay numeric.
// CHECK: [NodeMaxDispatchGrid(4, 2, 1)]
// CHECK: [numthreads(2, 1, 1)]
// CHECK: void main(
