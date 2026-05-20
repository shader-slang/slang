//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An InitializerListExpr in array context populates the array elements positionally.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says InitializerListExpr is `{ a, b, c }`. Used for an
// array, the elements end up at positions 0..N-1 in order.

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
