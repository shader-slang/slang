//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=074addd0c2d66598101cf0dfc70be7e1bcffe641d2e7c455a491f14df2014a84
//META: purpose=When narrowing leaves more than one candidate for a context that needs exactly one, the checker reports an ambiguous reference (E39999) with one candidate note per surviving item.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ambiguity failure mode on a value reference. Two
// sibling-injected namespaces each contribute a constant named VAL, and
// a value context cannot hold an overloaded result, so both survive
// narrowing and the checker diagnoses. CHECK annotates the error at the
// use site and each candidate note, which also pins that lookup handed
// the caller both items rather than picking one itself.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

namespace A { static const int VAL = 1; }
//CHECK:                       ^^^ candidate: static const int A::VAL
namespace B { static const int VAL = 2; }
//CHECK:                       ^^^ candidate: static const int B::VAL

using namespace A;
using namespace B;

void main()
{
    int z = VAL;
//CHECK:    ^^^ ambiguous reference to 'VAL'
}
