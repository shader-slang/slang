//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
//META: purpose=When no candidate in the overload set matches the call's argument count and types, the call is rejected with a diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The overload-resolution doc says the call is rejected if no
// candidate succeeds. Verify the "not enough arguments" diagnostic
// when the call passes one argument to a two-argument function.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void foo(int a, int b) {}

void main()
{
    foo(1);
/*CHECK:
       ^ not enough arguments to call
       ^ not enough arguments to call (got 1, expected 2)
*/
//CHECK: E39999
}
