//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
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
