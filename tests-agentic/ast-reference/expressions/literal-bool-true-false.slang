//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=A BoolLiteralExpr carries `true` or `false`; both spellings evaluate to their respective boolean values.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The keywords `true` and `false` parse as BoolLiteralExpr nodes
// whose `value: bool` field is the corresponding boolean. Using them
// in conditionals selects the matching branch.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int t = true  ? 11 : 22;
    int f = false ? 11 : 22;
    // CHECK: t=11
    printf("t=%d\n", t);
    // CHECK: f=22
    printf("f=%d\n", f);
}
