//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=ce44867411acd029099bae586609553fe57570c9431906d8d42a8d17c424b6bc
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
}
