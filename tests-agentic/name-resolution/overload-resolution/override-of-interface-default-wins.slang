//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=cf4ad761e64c7c27fa923b073fa264eed8e3a5152ac4e063d5e36eba8cb6323b
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
