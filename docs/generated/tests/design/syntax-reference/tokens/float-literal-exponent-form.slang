//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
//META: purpose=Boundary - a FloatingPointLiteral with a decimal exponent (`1e3`) is a single token; the `e` and digits are part of the literal text.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: `1e3` should decode to 1000.0. If the lexer split
// the `e` off, `e3` would be an identifier and the expression would
// fail to type-check.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float v = 1e3;
    // CHECK: v=1000
    printf("v=%d\n", int(v));
}
