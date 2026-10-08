//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=b6c75281223e78050e4c294d358e1758fb007b265f058561dae282641f1095c2
//META: purpose=A `private` field is not visible outside its enclosing aggregate; access from a free function is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per-keyword semantics: `private` is visible only inside the same
// aggregate. A free function cannot reach a private field; lookup
// finds the candidate then `filterLookupResultByVisibilityAndDiagnose`
// rejects it with `declaration not accessible`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    private int secret;
}

void test(S s)
{
    int x = s.secret;
/*CHECK:
              ^^^^^^ declaration not accessible
              ^^^^^^ 'secret' is not accessible from the current context.
*/
// CHECK: E30600
}
