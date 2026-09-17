//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=c2efc22fd6b393971554625ec6e5d1342ed8b02f96b9afdf61ab48806b3d700e
//META: purpose=Negative - an integer literal followed by a non-documented suffix (`xyz`) triggers the "invalid suffix on integer literal" diagnostic.
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe: the doc says suffix characters like `u`, `l`, `f`,
// `h` are part of the literal token; an unrecognized suffix
// (`xyz`) must be rejected with an explicit diagnostic from the
// checker.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 1xyz;
//CHECK:    ^^^^ invalid suffix on integer literal
//CHECK:    ^^^^ invalid suffix 'xyz' on integer literal
}
