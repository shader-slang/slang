//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:39:00Z
//META: source_commit=199a52d71d360a84e8a13837b913746113a9b510
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=28a5007e7990e447eb6cc6141908413f26bb0721bb1b7dcb8e454fd24d38dbb8
//META: purpose=Boundary - a leading-zero exponent with a `-` sign (`0e-3`) lexes as a single FloatingPointLiteral token.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative-exponent spelling of the leading-zero rule:
// in `probe(0e-3)` the `-` and digits are part of one float token. If
// the lexer split at the `-`, the expression would parse as `0e - 3`
// and fail on the invalid `e` suffix; the float overload firing
// (kind=1) proves the single-token reading.

//TEST:INTERPRET(filecheck=CHECK):

int probe(float x) { return 1; }
int probe(int x) { return 2; }

void main()
{
    // CHECK: kind=1
    printf("kind=%d\n", probe(0e-3));
}
