//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=a916857bbe07c220fc4b183b2a1794d39a799be9e41fa992dc6a49d37be94635
//META: purpose=When no focused reason is recorded the inference failure falls through to the blanket could-not-specialize-generic diagnostic naming the argument types.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Kind::None arm of the inference-failure switch. Nothing pins the
// pack-count parameter N here, so no focused reason is captured and the
// finalize phase emits the generic fallback. CHECK pins that fallback together
// with the argument-type list it renders.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void load<let N : int, each TIndex>(TIndex indices) where countof(TIndex) == N
{
}

void main()
{
    load(1, 2, 3);
//CHECK:^ could not specialize generic for arguments of type (int, int, int)
}
