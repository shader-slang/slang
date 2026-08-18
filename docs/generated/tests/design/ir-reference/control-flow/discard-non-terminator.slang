//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#discard
//META: doc_section_digest=0772e4d9559e7c0aa30a097a054020db1d3ad67e21b2e96e8c76633db7bb3b82
//META: purpose=`discard` is not a terminator: it sits as an ordinary instruction inside a block, and the block's real terminator (unconditionalBranch to the post-if merge) follows.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A pixel-shader discard inside an if-arm appears in the IR dump as
// a `discard` instruction followed by an `unconditionalBranch` —
// the branch, not the discard, is the block's terminator.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage fragment

uniform float threshold;

float4 main(float4 pos : SV_Position) : SV_Target
{
    if (pos.x < threshold)
        discard;
    return float4(1, 0, 0, 1);
}

// CHECK-LABEL: func %main
// CHECK: discard
// CHECK-NEXT: unconditionalBranch(%{{[0-9]+}})
