//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
