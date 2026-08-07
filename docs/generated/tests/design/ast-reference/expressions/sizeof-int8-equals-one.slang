//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies the lower-edge boundary of SizeOfExpr: `sizeof(int8_t)` is the documented value 1.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for SizeOfExpr: the smallest documented
// numeric type is 1 byte wide.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = int(sizeof(int8_t));
    // CHECK: s=1
    printf("s=%d\n", s);
}
