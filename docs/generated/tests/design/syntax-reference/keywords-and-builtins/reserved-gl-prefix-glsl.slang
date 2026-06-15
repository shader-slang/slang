//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#reserved-identifier-prefixes
//META: doc_section_digest=3f95ec9865b25008e28f60ca66efd13758cbf3599f18b4c7e50e7d0178187f2f
//META: purpose=`gl_`-prefixed identifiers stand for shader-stage built-ins supplied by `glsl.meta.slang`; the GLSL emit uses the gl_-prefixed spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Write to the vertex-stage position output and verify the GLSL emit
// names it `gl_Position` (the GLSL spelling for `SV_Position`).

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage vertex

float4 main(float3 pos) : SV_Position
{
    //CHECK: gl_Position
    return float4(pos, 1.0);
}
