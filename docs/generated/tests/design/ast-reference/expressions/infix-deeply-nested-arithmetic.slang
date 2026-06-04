//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=Stresses InfixExpr parsing with a five-deep paren-nested arithmetic chain: ((((1+2)*3)-4)/5) must evaluate to 1.
//META: intent=stress
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Deep nesting stress for InfixExpr / ParenExpr interaction. Five
// levels of parentheses force the parser through repeated descent;
// the result must still equal the documented arithmetic value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int r = ((((1 + 2) * 3) - 4) / 5);
    // CHECK: r=1
    printf("r=%d\n", r);
}
