//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=764e196b19a5dd0e0f2c516c498be250f0fdaba7e58c28f2ac820f88be1e5466
//META: purpose=CompareLookupResultItems prefers a concrete member over the interface requirement it satisfies, so a conforming type's override wins over the interface default.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the concrete-member-over-interface-requirement arm. Through a
// generic bounded by IHasV both the interface's default body and Impl's
// override are reachable; CHECK asserts the override's value, which is what
// this tie-break selects.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasV
{
    int v() { return 1; }
}

struct Impl : IHasV
{
    override int v() { return 99; }
}

int call<T : IHasV>(T t) { return t.v(); }

void main()
{
    Impl i;
    int r = call(i);
    // CHECK: r=99
    printf("r=%d\n", r);
}
