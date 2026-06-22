//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#thistype-and-assoctypedecl
//META: doc_section_digest=0785b0b0a350abfe71a92e3bb11a7dfa2b2a2496738ceca31b6310367f428e33
//META: purpose=A ThisType inside an interface stands for the conforming type; a method whose return type is `This` returns the concrete type when dispatched on a concrete value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ThisType` is the type of the `This` value inside an interface
// body. The user-facing claim: a `This`-returning method, dispatched
// on a concrete type, returns a value of that concrete type — the
// returned `This` substitutes to the conforming type at dispatch.

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
