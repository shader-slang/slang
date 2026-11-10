//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=a94dca0039eb4252e1b8fd3b7f350745e39efaac5fece1dae4ad82b3e6325392
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
