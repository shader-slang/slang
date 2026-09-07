//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=a916857bbe07c220fc4b183b2a1794d39a799be9e41fa992dc6a49d37be94635
//META: purpose=A pack argument whose length violates a countof constraint fails as VariadicPackCountMismatch during inference and is reported with the expected and actual element counts.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a pack-count violation is not reported as an ordinary arity
// error: the constraint is solved as part of generic-argument inference, so the
// candidate stalls at GenericArgumentInferenceFailed and the finalize phase
// prints the captured counts. CHECK pins the message naming both counts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void load<let N : int, each TIndex>(TIndex indices) where countof(TIndex) == N
{
}

void main()
{
    load<2>(1, 2, 3);
//CHECK:   ^ expected 2 elements, but pack argument has 3
}
