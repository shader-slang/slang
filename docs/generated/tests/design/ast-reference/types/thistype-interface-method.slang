//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#thistype-and-assoctypedecl
//META: doc_section_digest=5f248bb7696458fa65ee7706db2d88769ea1a0b81f789dadf95e485871c7d498
//META: purpose=A ThisType inside an interface stands for the conforming type; a method whose return type is `This` returns the concrete type when dispatched on a concrete value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies ThisType is the type of `this` inside an interface body:
// a `This`-returning method dispatched on a concrete type returns a
// value of that concrete type, so a follow-up concrete method call
// succeeds. CHECK asserts the cloned value's mark, proving `This`
// substituted to the conforming type.

//TEST:INTERPRET(filecheck=CHECK):

interface ICloneable
{
    This makeClone();
    int getMark();
}

struct Widget : ICloneable
{
    int mark;
    This makeClone() { Widget w; w.mark = this.mark + 100; return w; }
    int getMark() { return mark; }
}

int cloneAndRead<T : ICloneable>(T v)
{
    T c = v.makeClone();
    return c.getMark();
}

void main()
{
    Widget w;
    w.mark = 5;
    // CHECK: m=105
    printf("m=%d\n", cloneAndRead(w));
}
