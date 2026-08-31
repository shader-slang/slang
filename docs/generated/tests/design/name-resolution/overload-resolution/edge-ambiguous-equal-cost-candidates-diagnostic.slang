//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=7aaa4b55b90bf9dc8832d907095aad278bba32f9533b97e07d8e09e58134c060
//META: purpose=Two Applicable candidates that tie on every comparator step are both kept in bestCandidates and reported as an ambiguous call.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ambiguity path. Two same-signature functions live in sibling
// namespaces that are both brought into scope, so cost, specificity and scope
// distance all tie and the comparator returns equal. CHECK pins the ambiguous
// call error at the call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

namespace A { int act(int x) { return 1; } }
namespace B { int act(int x) { return 2; } }

using namespace A;
using namespace B;

void main()
{
    int v = act(7);
//CHECK:       ^ ambiguous call to 'act'
    printf("%d\n", v);
}
