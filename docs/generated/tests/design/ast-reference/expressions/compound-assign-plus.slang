//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=A compound-assignment InfixExpr `a += b` updates `a` to `a + b`; the resulting value is the sum.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The InfixExpr Nodes row covers binary operators including the
// compound-assignment forms. `+=` updates the l-value in place; the
// read after the statement returns the updated value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 10;
    x += 5;
    // CHECK: x=15
    printf("x=%d\n", x);
}
