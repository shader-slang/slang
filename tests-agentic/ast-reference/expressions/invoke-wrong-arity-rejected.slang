//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification
//META: doc_section_digest=159873da063c0de0bb81f96fd4bec64b70eb1664f509af0a10c8eee0b3e19e7f
//META: purpose=An InvokeExpr whose argument-list length does not match any candidate is rejected with a "not enough arguments to call" diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative shape for the InvokeExpr / overload-resolution contract:
// the only candidate takes two parameters; passing one must be
// diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void foo(int a, int b) { }

void main()
{
    foo(1);
/*CHECK:
       ^ not enough arguments to call
       ^ not enough arguments to call (got 1, expected 2)
*/
//CHECK: E39999
}
