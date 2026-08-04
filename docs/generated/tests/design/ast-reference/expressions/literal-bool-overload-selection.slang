//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
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
