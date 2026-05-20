//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification
//META: doc_section_digest=159873da063c0de0bb81f96fd4bec64b70eb1664f509af0a10c8eee0b3e19e7f
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
