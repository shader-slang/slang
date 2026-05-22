//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#overloadedexpr-and-overloadedexpr2
//META: doc_section_digest=d699406eb906de5a0cae4283d7dc36a9700ff64991d7f63e2cef9b7d64bb9a71
//META: purpose=When the call's argument type does not match any candidate parameter type, the InvokeExpr is rejected (the overload set collapse fails to find a match).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says OverloadedExpr is collapsed during overload
// resolution; if no candidate applies, the checker reports an error.
// The argument type does not match the only candidate; the
// InvokeExpr cannot resolve.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int v; }

void foo(int a) { }

void notMain()
{
    S s;
    foo(s);
/*CHECK:
        ^ type mismatch in expression
        ^ expected an expression of type 'int', got 'S'
*/
// CHECK: E30019
}
