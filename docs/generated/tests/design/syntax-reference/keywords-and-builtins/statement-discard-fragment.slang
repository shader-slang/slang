//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=d6e74209f0cfcbd939127caa9bf2791387db9155c2d0b2f0a561349e2e185456
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
