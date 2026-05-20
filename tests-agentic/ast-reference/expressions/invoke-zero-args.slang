//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification
//META: doc_section_digest=159873da063c0de0bb81f96fd4bec64b70eb1664f509af0a10c8eee0b3e19e7f
//META: purpose=Verifies an InvokeExpr with zero arguments — `f()` — calls the function and returns its result.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for InvokeExpr.arguments: the empty list.
// `f()` must construct an InvokeExpr with no args and dispatch
// correctly to the zero-arg overload.

//TEST:INTERPRET(filecheck=CHECK):

int five() { return 5; }

void main()
{
    // CHECK: r=5
    printf("r=%d\n", five());
}
