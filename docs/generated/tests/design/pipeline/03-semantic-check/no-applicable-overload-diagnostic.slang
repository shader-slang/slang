//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
//META: purpose=When no candidate in the overload set matches the call's argument count and types, the call is rejected with a diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The overload-resolution doc says the call is rejected if no
// candidate succeeds. Verify the "not enough arguments" diagnostic
// when the call passes one argument to a two-argument function.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void foo(int a, int b) {}
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
