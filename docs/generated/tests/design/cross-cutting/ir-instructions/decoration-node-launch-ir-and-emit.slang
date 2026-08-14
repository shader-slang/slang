//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=af2eb87d563403d52d71a7d33122c279b70c033339e51393dbe00f9f5f506a59
//META: purpose=The `nodeLaunch` decoration carries the launch mode as a string literal so HLSL emission re-spells it as a quoted attribute string rather than an integer.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the work-graph decoration row's specific claim that
// `nodeLaunch`'s `mode` operand is an `IRStringLit`, not an enum ordinal.
// The IR CHECK asserts the decoration prints as `nodeLaunch("broadcasting")`
// — a quoted string in the dump — and the HLSL CHECK asserts the emitted
// attribute is `[NodeLaunch("broadcasting")]`. If the mode were carried as
// an integer, the emitted attribute would have to be reconstructed from a
// numeric mapping and a round-trip of the spelling could not be asserted.
// The sibling `nodeDispatchGrid` row is checked alongside because it is the
// integer-operand counterpart in the same decoration group.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage node -entry myNode -profile lib_6_8 -experimental-feature -dump-ir -o /dev/null
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage node -entry myNode -profile lib_6_8 -experimental-feature

import experimental.workgraph;

struct MyRecord { uint value; };

RWStructuredBuffer<uint> outbuf;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[numthreads(1, 1, 1)]
void myNode(DispatchNodeInputRecord<MyRecord> input)
{
    outbuf[0] = input.Get().value;
}

// IR-DAG: [nodeLaunch("broadcasting")]
// IR-DAG: [nodeDispatchGrid(2 : Int, 1 : Int, 1 : Int)]

// HLSL-DAG: {{\[}}NodeLaunch("broadcasting"){{\]}}
// HLSL-DAG: {{\[}}NodeDispatchGrid(2, 1, 1){{\]}}
