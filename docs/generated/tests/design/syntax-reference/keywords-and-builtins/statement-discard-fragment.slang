//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
//META: purpose=`discard` is a fragment-shader statement keyword; it terminates fragment processing for the current invocation.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verify the parser accepts `discard` as a statement and the HLSL
// backend emits the corresponding token in the generated text. This
// is target-dependent text-emit behavior; HLSL is enough to anchor
// the parse-level claim.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage fragment

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    if (uv.x < 0.0) { discard; }
    //CHECK: discard
    return float4(1, 0, 0, 1);
}
