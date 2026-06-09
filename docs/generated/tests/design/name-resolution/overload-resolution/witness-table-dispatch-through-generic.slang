//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#operator-overloading
//META: doc_section_digest=017e2a9ca7d5b435c5665a1115eba51fbc2b30352078a8ab4c0e65601bfa2273
//META: purpose=Witness-table dispatch through an interface-bounded generic picks the conforming type's implementation of the interface method, observable as the conforming-type-specific return value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An interface method called through a generic T:IShape dispatches
// through T's witness table to the conforming type's
// implementation. Each conforming type returns a distinct tag.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    int kind();
}

struct Square : IShape
{
    int kind() { return 4; }
}

struct Triangle : IShape
{
    int kind() { return 3; }
}

int describe<T : IShape>(T s)
{
    return s.kind();
}

void main()
{
    Square sq;
    Triangle tr;
    int a = describe(sq);
    int b = describe(tr);
    // CHECK: a=4 b=3
    printf("a=%d b=%d\n", a, b);
}
