//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#invokeexpr-and-the-call--operator--cast-unification
//META: doc_section_digest=bed8880e5d3d3d67324d1cc51b8508d2d1f9ce1f6e5e3f3eadf4c071f728a402
//META: purpose=An InvokeExpr whose argument-list length does not match any candidate is rejected with a "not enough arguments to call" diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative shape for the InvokeExpr / overload-resolution contract:
// the only candidate takes two parameters; passing one must be
// diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void foo(int a, int b) { }
/*CHECK:
     ^^^ candidate: func foo(int, int) -> void
*/

void main()
{
    foo(1);
/*CHECK:
       ^ not enough arguments to call
       ^ not enough arguments to call (got 1, expected 2)
*/
}
