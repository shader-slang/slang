//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=An InitializerListExpr in array context populates the array elements positionally.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a brace initializer applied to an array binds its elements
// positionally. Three distinct values are listed and each position is read
// back separately, so a reordering or off-by-one placement fails a CHECK.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[3] = {100, 200, 300};
    // CHECK: a0=100
    printf("a0=%d\n", arr[0]);
    // CHECK: a1=200
    printf("a1=%d\n", arr[1]);
    // CHECK: a2=300
    printf("a2=%d\n", arr[2]);
}
