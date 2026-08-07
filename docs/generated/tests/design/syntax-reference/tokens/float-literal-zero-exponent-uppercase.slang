//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:39:00Z
//META: source_commit=199a52d71d360a84e8a13837b913746113a9b510
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=28a5007e7990e447eb6cc6141908413f26bb0721bb1b7dcb8e454fd24d38dbb8
//META: purpose=Boundary - an uppercase exponent marker after a bare zero (`0E5`) still lexes as a single FloatingPointLiteral.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the uppercase spelling of the leading-zero exponent rule:
// `0E5` must lex as one floating-point token exactly like `0e5`. The
// overload probe returns 1 only when the literal's type is a float;
// if `E5` were treated as an integer suffix the program would be
// rejected before the probe could run.

//TEST:INTERPRET(filecheck=CHECK):

int probe(float x) { return 1; }
int probe(int x) { return 2; }

void main()
{
    // CHECK: kind=1
    printf("kind=%d\n", probe(0E5));
}
