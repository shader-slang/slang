//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#invokeexpr-and-the-call--operator--cast-unification
//META: doc_section_digest=bed8880e5d3d3d67324d1cc51b8508d2d1f9ce1f6e5e3f3eadf4c071f728a402
//META: purpose=Stresses InvokeExpr.arguments with eight positional arguments — the call must dispatch and the eight values must arrive in order.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress for InvokeExpr argument-list length. Eight ints arrive
// positionally; their sum is 36 (1+...+8). Ordering matters: the
// argument-position bookkeeping must survive into the call.

//TEST:INTERPRET(filecheck=CHECK):

int sum8(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return a + b + c + d + e + f + g + h;
}

void main()
{
    // CHECK: s=36
    printf("s=%d\n", sum8(1, 2, 3, 4, 5, 6, 7, 8));
}
