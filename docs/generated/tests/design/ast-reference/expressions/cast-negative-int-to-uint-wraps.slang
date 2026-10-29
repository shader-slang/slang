//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies that the TypeCastExpr `uint(-1)` wraps to 0xFFFFFFFF — the documented two's-complement reinterpretation across the sign boundary.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sign-boundary cast: a signed -1 becomes the unsigned MAX. The
// TypeCastExpr must preserve the bit pattern across the conversion.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int neg = -1;
    uint u = uint(neg);
    // CHECK: u=4294967295
    printf("u=%u\n", u);
}
