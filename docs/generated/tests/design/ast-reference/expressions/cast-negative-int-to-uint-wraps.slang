//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
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
