//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=4c78ab08cf659f53d04e87356fa441615410f33a23152c70e9fb93b377efd998
//META: purpose=A result whose first item is a namespace is exempted from the ambiguity report, so a name reachable as two same-named namespaces stays usable as a qualifier.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the namespace carve-out in the ambiguity path. Two outer
// namespaces are injected as siblings and each contains a namespace Q,
// so an unqualified `Q` yields two items — the exact shape that
// diagnoses for values in
// ambiguous-reference-from-two-using-namespaces.slang. CHECK asserts
// 1 + 2, so both qualified calls resolved and no ambiguity was
// reported for the overloaded namespace reference itself.

//TEST:INTERPRET(filecheck=CHECK):

namespace P { namespace Q { int f() { return 1; } } }
namespace R { namespace Q { int g() { return 2; } } }

using namespace P;
using namespace R;

void main()
{
    // CHECK: r=3
    printf("r=%d\n", Q::f() + Q::g());
}
