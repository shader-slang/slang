//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#discard
//META: doc_section_digest=9148728e2771b240bdfccaa8dd48eb1252c215ce162b5be9a123df22827fe577
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
