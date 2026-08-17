//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#attributes-and-decorations
//META: doc_section_digest=15532574d330d6144dfcff6e6eb0a667f59639395aa4b4eccbd806a31f935008
//META: purpose=`Attribute` parses as `'[' AttributeBody ']'` where `AttributeBody` is `AttributeName '(' ArgList ')'`, and attaches to the following decl.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bracket-form attribute production. `[numthreads(1,1,1)]`
// is an `AttributeName` followed by an `ArgList` inside parens, inside
// square brackets, attached to the entry-point function.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: numthreads
}
