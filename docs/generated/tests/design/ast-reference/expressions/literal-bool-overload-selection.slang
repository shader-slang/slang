//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=1e8f144399f6073f93949619c72c2230cfb4349def0ab49ac5088e68f85aa4ab
//META: purpose=Verifies that a BoolLiteralExpr `true`/`false` is typed as `bool` and steers overload resolution to the bool candidate (not to int).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Type-level observation of BoolLiteralExpr. Two overloads — one
// for int and one for bool — let the literal pick the bool path.

//TEST:INTERPRET(filecheck=CHECK):

int which(int v)  { return 1; }
int which(bool v) { return 2; }

void main()
{
    // CHECK: t=2
    printf("t=%d\n", which(true));
    // CHECK: f=2
    printf("f=%d\n", which(false));
}
