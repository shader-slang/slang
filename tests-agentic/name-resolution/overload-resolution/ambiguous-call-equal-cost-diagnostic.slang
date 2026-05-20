//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
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
}
