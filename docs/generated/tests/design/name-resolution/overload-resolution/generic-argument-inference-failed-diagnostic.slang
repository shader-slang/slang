//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-06-24T09:20:31Z
//META: source_commit=b25e06aeb8862d207541c3d812c57cb9e381b802
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
//META: purpose=When TryCheckGenericOverloadCandidateTypes cannot infer T from the argument shape, the candidate ends at Status::GenericArgumentInferenceFailed and CompleteOverloadCandidate emits generic-argument-inference-failed (could not infer generic argument for parameter).
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
/*CHECK:
  ^^^^^ see declaration of func unbox<T> -> T
*/

void main()
{
    int x = unbox(42);
//CHECK:         ^ could not infer generic argument for parameter 'T'
}
