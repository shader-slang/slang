//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=7aaa4b55b90bf9dc8832d907095aad278bba32f9533b97e07d8e09e58134c060
//META: purpose=When several non-applicable candidates tie for best the resolver emits no-applicable-overload-for-name-with-args directly, listing the argument types it could not match.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the multi-candidate failure path. Neither overload accepts a user
// struct, so both stall at the type step and tie for best, which is exactly the
// shape that bypasses the finalize re-check and reports the aggregate error.
// CHECK pins that error together with the argument-type list it prints.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Custom { int v; }

int handle(int x)   { return 1; }
int handle(float x) { return 2; }

void main()
{
    Custom c;
    int t = handle(c);
//CHECK:          ^ no overload for 'handle' applicable to arguments of type (Custom)
    printf("%d\n", t);
}
