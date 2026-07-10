//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:39:00Z
//META: source_commit=199a52d71d360a84e8a13837b913746113a9b510
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=28a5007e7990e447eb6cc6141908413f26bb0721bb1b7dcb8e454fd24d38dbb8
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
