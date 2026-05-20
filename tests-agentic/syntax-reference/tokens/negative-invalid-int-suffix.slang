//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/llm-generated/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
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
