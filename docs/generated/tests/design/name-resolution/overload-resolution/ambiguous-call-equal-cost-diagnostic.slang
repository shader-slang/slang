//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
//META: purpose=Two Applicable candidates with identical cost / specificity / scope produce ambiguous-overload-for-name-with-args (39999); the resolver emits the diagnostic with the name and a candidate note per tied entry.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Two Applicable candidates with identical cost / specificity /
// scope. CompareOverloadCandidates returns 0; the candidate is
// appended to context.bestCandidates. After the probe phase the
// caller emits ambiguous-overload-for-name-with-args".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

namespace A { int act(int x) { return 1; } }
namespace B { int act(int x) { return 2; } }

using namespace A;
using namespace B;

void main()
{
    int v = act(7);
//CHECK:       ^ ambiguous call to 'act'
//CHECK: E39999
}
