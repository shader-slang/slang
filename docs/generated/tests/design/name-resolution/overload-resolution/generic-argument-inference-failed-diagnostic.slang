//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
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
/*CHECK:
  ^^^^^ see declaration of func unbox<T> -> T
*/

void main()
{
    int x = unbox(42);
//CHECK:         ^ could not infer generic argument for parameter 'T'
}
