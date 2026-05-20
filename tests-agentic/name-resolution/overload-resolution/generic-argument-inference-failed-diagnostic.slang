//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
//META: purpose=When TryCheckGenericOverloadCandidateTypes cannot infer T from the argument shape, the candidate ends at Status::GenericArgumentInferenceFailed and CompleteOverloadCandidate emits generic-argument-inference-failed (could not specialize generic for arguments of type).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Generic-argument inference failure. The candidate ends at
// Status::GenericArgumentInferenceFailed. CompleteOverloadCandidate
// emits generic-argument-inference-failed".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Box<T>
{
    T inner;
}

T unbox<T>(Box<T> b) { return b.inner; }

void main()
{
    int x = unbox(42);
//CHECK:         ^ could not specialize generic for arguments of type (int)
}
