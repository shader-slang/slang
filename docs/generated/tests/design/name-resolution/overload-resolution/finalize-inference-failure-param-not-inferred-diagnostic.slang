//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=a916857bbe07c220fc4b183b2a1794d39a799be9e41fa992dc6a49d37be94635
//META: purpose=When a candidate ends at GenericArgumentInferenceFailed with OrdinaryGenericParamNotInferred, CompleteOverloadCandidate emits the focused could-not-infer-parameter diagnostic naming the parameter.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the focused per-Kind diagnostic selection in the finalize phase.
// `unbox` needs T from a `Box<T>` argument, but the call passes a bare int, so
// T stays undetermined. CHECK pins the parameter-specific message rather than
// the blanket "could not specialize generic" fallback.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Box<T> { T inner; }

T unbox<T>(Box<T> b) { return b.inner; }

void main()
{
    int x = unbox(42);
//CHECK:         ^ could not infer generic argument for parameter 'T'
    printf("%d\n", x);
}
