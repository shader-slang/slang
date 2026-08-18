//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=98ff1b59e664e45b3e67fde56e5d2482e7d7845bb990f58cbca3a8c2edfcdd84
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
