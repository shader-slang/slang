//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=e78e1fa50e5d6d7678c40996a6f91cc20ba30ced884ab9bc2c4a154f6aba1dfc
//META: purpose=The parser attaches an attribute wherever modifier parsing runs and leaves the question of which declarations it may modify to semantic checking, which reports an invalid placement.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the division of labour between the two stages. `[unroll]` is a loop
// attribute, but here it is written in front of a function declaration. The
// parser raises no complaint — there is no syntax error and no unexpected-token
// report — and the only diagnostic is the placement error the checker raises
// once it knows what kind of declaration the modifier landed on. Exhaustive
// mode makes the absence of a parse-stage diagnostic part of the assertion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[unroll]
/*CHECK:
 ^^^^^^ error E31002
 ^^^^^^ attribute 'unroll' is not valid here
*/
void main()
{
}
