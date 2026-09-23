//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#invokeexpr-and-the-call--operator--cast-unification
//META: doc_section_digest=bed8880e5d3d3d67324d1cc51b8508d2d1f9ce1f6e5e3f3eadf4c071f728a402
//META: purpose=A direct call `f(x)` parses as an InvokeExpr whose functionExpr is the callee and whose arguments are the argument list; the call returns the callee's result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `add(2, 3)` is the canonical InvokeExpr: the function expression is
// `add` and the argument list is `[2, 3]`. The result, 5, demonstrates
// that the call resolved and returned correctly.

//TEST:INTERPRET(filecheck=CHECK):

int add(int a, int b) { return a + b; }

void main()
{
    // CHECK: r=5
    printf("r=%d\n", add(2, 3));
}
