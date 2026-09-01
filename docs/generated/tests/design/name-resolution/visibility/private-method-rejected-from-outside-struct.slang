//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=7d96ab9817d6f6251e2148b7d4d9959fb9ec73041354e919db537f46458d416e
//META: purpose=A `private` method is not visible outside its enclosing aggregate; an overload call from a free function is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per-keyword semantics: `private` methods are filtered out by
// `TryCheckOverloadCandidateVisibility` during overload resolution.
// A free function attempting the call gets `not accessible`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    private int helper() { return 1; }
}

void test(S s)
{
    int x = s.helper();
/*CHECK:
              ^^^^^^ declaration not accessible
              ^^^^^^ 'helper' is not accessible from the current context.
*/
// CHECK: E30600
}
