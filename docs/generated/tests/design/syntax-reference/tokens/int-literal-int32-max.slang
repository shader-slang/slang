//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Boundary - `2147483647` (int32 MAX) is a valid IntegerLiteral that fits in `int` without overflow.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the literal that exactly matches `INT_MAX` for a
// 32-bit signed integer lexes as an `IntegerLiteral` and prints as
// the expected value via the byte-code interpreter.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 2147483647;
    // CHECK: v=2147483647
    printf("v=%d\n", v);
}
