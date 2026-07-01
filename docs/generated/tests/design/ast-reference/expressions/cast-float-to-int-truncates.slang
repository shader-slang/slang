//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=Verifies that the TypeCastExpr from float to int truncates toward zero for positive and negative finite values.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Float-to-int boundary: truncation (round-toward-zero) for both
// signs. 3.7 -> 3, -3.7 -> -3.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float v = 3.7f;
    int i = int(v);
    // CHECK: i=3
    printf("i=%d\n", i);
    float nv = -3.7f;
    int ni = int(nv);
    // CHECK: ni=-3
    printf("ni=%d\n", ni);
}
