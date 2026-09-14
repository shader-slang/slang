//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
//META: purpose=Boundary - a leading-zero exponent with an explicit `+` sign (`0e+1`) lexes as a single FloatingPointLiteral token.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the exponent's sign character is consumed into the
// leading-zero float token: in `probe(0e+1)` the `+` must belong to
// the literal, not act as a binary operator between `0e` and `1`.
// A binary-operator reading would be an invalid-suffix error on `0e`;
// the float overload firing (kind=1) proves the one-token reading.

//TEST:INTERPRET(filecheck=CHECK):

int probe(float x) { return 1; }
int probe(int x) { return 2; }

void main()
{
    // CHECK: kind=1
    printf("kind=%d\n", probe(0e+1));
}
