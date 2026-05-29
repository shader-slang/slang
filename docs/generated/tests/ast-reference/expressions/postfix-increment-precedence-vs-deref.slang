//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=Verifies that a PostfixExpr `a[i]++` increments the indexed array element (not the index): the IndexExpr is the operand of the PostfixExpr.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on postfix-vs-index parser precedence. `a[i]++` must
// bind `++` to the IndexExpr, not to `i`. After the operation,
// `a[1]` should be one greater than the original.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[4] = {10, 20, 30, 40};
    int prev = a[1]++;
    // CHECK: prev=20
    printf("prev=%d\n", prev);
    // CHECK: now=21
    printf("now=%d\n", a[1]);
}
