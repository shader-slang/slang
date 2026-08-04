//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=dfab71819770e1c3116c8ab890c4c05608d82d83fe8f00436fcbffa88f52900d
//META: purpose=CompareLookupResultItems (comparator step 3) prefers an override of an inherited member over the inherited member itself; a conforming-type override of an interface default fires through a generic dispatch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The interface provides a default; the conforming struct overrides
// it. Through a generic call site the overriding implementation
// (return 99) wins over the default (return 1).

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
