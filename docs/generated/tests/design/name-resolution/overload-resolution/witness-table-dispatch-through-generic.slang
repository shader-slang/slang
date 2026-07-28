//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#operator-overloading
//META: doc_section_digest=6c67481fccb6a5d96551c93af7509ffafa480d11451126832bec4a0352c4b938
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
