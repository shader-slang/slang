//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#module-and-namespace
//META: doc_section_digest=cc12c20b9797d05d866e6ea2e2ea4412387e14ae52059ede92e3c563abaeca67
//META: purpose=The target of a using declaration is attached to the scope chain as a sibling, so the namespace's names become reachable unqualified at the use site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the sibling-chain half of the namespace machinery. A
// namespace cannot be merged into the enclosing container, so `using`
// splices it in as a sibling and the sibling loop of the scope walk is
// what reaches it. CHECK asserts 42 from an unqualified call, which
// would be undefined without that splice.

//TEST:INTERPRET(filecheck=CHECK):

namespace Foo
{
    int answer() { return 42; }
}

using namespace Foo;

void main()
{
    int v = answer();
    // CHECK: v=42
    printf("v=%d\n", v);
}
