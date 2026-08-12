//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=An IndexExpr `a[i]` reads the i-th element of an indexed value; with a baseExpression and one index expression.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// IndexExpr is documented as `a[i]` with baseExpression and
// indexExprs. Indexing a fixed-size array returns the stored element.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[4] = {10, 20, 30, 40};
    // CHECK: a0=10
    printf("a0=%d\n", arr[0]);
    // CHECK: a3=40
    printf("a3=%d\n", arr[3]);
}
