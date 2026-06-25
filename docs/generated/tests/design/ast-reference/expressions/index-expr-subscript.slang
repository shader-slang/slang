//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
