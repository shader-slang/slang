//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
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
