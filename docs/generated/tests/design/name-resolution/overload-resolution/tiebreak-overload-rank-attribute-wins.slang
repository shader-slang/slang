//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=1d384c53a44075a9ff7b3de0f1907c30fb579e521154ac25ad145d433be8dd6a
//META: purpose=The [OverloadRank(N)] attribute breaks a tie that every earlier comparator step leaves equal, and the higher rank wins.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the final overload-rank step of the comparator. The two candidates
// are identical in signature and live in sibling namespaces, so cost,
// specificity and scope distance all tie; the only difference is the rank
// attribute. CHECK asserts the ranked candidate's tag and, by the call
// resolving at all, that the tie was broken rather than reported ambiguous.

//TEST:INTERPRET(filecheck=CHECK):

namespace A { [OverloadRank(1)] int act(int x) { return 1; } }
namespace B { int act(int x) { return 2; } }

using namespace A;
using namespace B;

void main()
{
    int tag = act(7);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
