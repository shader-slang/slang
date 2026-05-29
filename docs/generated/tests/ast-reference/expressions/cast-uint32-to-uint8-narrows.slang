//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
