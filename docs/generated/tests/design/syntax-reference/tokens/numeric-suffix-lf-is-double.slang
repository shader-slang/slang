//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Boundary - the `lf` suffix on a FloatingPointLiteral types it as `double` (64-bit float).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the `lf` (long-float) suffix on a floating-point
// literal is documented as part of the literal token. `3.5lf` should
// resolve to `double` while `3.5f` resolves to `float`.

//TEST:INTERPRET(filecheck=CHECK):

int kind(float v) { return 1; }
int kind(double v) { return 2; }

void main()
{
    // CHECK: kind=2
    printf("kind=%d\n", kind(3.5lf));
}
