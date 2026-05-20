//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=4430d3efb9516cd383356961351a223b29ad1c948e7c5ea9bf0a36d832a4b940
//META: purpose=A vertex-shader parameter declared with a non-SV semantic (POSITION) carries the same semantic decoration shape as SV-prefixed semantics with index -1 when no index is given.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The semantic decoration is shape-uniform across SV-prefixed and
// user-defined semantics: both encode as
// `semantic("NAME", index)`. POSITION (no explicit index)
// surfaces as `("POSITION", -1 : Int)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage vertex

[shader("vertex")]
float4 main(float3 pos : POSITION) : SV_Position
{
    return float4(pos, 1.0);
}

// CHECK: [semantic("POSITION", -1 : Int)]
