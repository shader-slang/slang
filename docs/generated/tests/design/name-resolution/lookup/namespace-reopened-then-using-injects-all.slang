//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#module-and-namespace
//META: doc_section_digest=cc12c20b9797d05d866e6ea2e2ea4412387e14ae52059ede92e3c563abaeca67
//META: purpose=Multiple same-named namespace blocks under one parent parse into a single namespace decl, so one using declaration makes members from every block reachable unqualified.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the parser-level merge rather than the sibling splice: the
// two `namespace M` blocks share a parent, so they must become one
// decl. A single `using namespace M` then has to expose members
// contributed by both blocks. CHECK asserts 1 + 2, which requires
// alpha and beta to have landed in the same namespace decl.

//TEST:INTERPRET(filecheck=CHECK):

namespace M
{
    int alpha() { return 1; }
}

namespace M
{
    int beta() { return 2; }
}

using namespace M;

void main()
{
    // CHECK: r=3
    printf("r=%d\n", alpha() + beta());
}
