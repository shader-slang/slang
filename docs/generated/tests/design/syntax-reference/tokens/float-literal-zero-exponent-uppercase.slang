//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=02a80201a5690a8a624d91ba229db896f43d7b71e5abd5c2e95980b3d576102d
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
