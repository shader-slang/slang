//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=The inheritance walk traverses every facet of the InheritanceInfo, including transitive super-interfaces, so a method declared on a top-most interface is reachable through a deeply derived constraint.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the inheritance walk iterates "InheritanceInfo::facets
// precomputed in slang-check-inheritance.cpp" — each facet is one
// path from the source type to a super-type. With IA <- IB <- IC and
// `Impl : IC`, the `aval()` method declared on IA must still be
// reachable through `Impl`'s facets via the transitive chain.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int aval(); }
interface IB : IA {}
interface IC : IB {}

struct Impl : IC
{
    int aval() { return 17; }
}

int via<T : IC>(T t) { return t.aval(); }

void main()
{
    Impl i;
    // CHECK: r=17
    printf("r=%d\n", via(i));
}
