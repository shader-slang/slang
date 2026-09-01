//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=A FloatingPointLiteralExpr carries a float-typed value; the suffix `lf` picks the double overload over float.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// FloatingPointLiteralExpr's suffixType decides which floating-point
// base type the literal denotes. The `lf` suffix denotes double; an
// unsuffixed `1.5f` (or `1.5`) denotes float. Pick the float overload
// for `1.5f` and the double overload for `1.5lf`.

//TEST:INTERPRET(filecheck=CHECK):

int which(float v)  { return 1; }
int which(double v) { return 2; }

void main()
{
    // CHECK: float=1
    printf("float=%d\n", which(1.5f));
    // CHECK: double=2
    printf("double=%d\n", which(1.5lf));
}
