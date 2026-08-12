//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies an IndexExpr at the lower-edge index 0 reads the first stored element.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for IndexExpr.indexExprs[0]. Reading `arr[0]`
// from a 4-element array must yield the documented first element.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[4] = {11, 22, 33, 44};
    // CHECK: v=11
    printf("v=%d\n", arr[0]);
}
