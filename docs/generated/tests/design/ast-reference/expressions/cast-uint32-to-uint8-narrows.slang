//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that an ExplicitCastExpr narrowing uint32 to uint8 keeps only the low 8 bits — the high bits set in 0xFF03 are dropped.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Narrowing cast: TypeCastExpr from a 32-bit source whose high byte
// is set must keep only the low 8 bits in the uint8 destination.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint v = 0xFF03u;
    uint8_t low = uint8_t(v);
    // CHECK: low=3
    printf("low=%d\n", int(low));
}
