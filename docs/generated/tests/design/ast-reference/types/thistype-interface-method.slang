//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#thistype-and-assoctypedecl
//META: doc_section_digest=35b2b46dc04e84b08b4aa611ac4530252370da8bdabd15d01fd1ac308064a09d
//META: purpose=A ThisType stands for the conforming type inside a polymorphic declaration, so a method returning `This` yields the concrete type when dispatched.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `This` is substituted with the conforming type rather
// than erased to the interface: the generic stores the result of a
// This-returning method in a `T` local and then calls a further
// requirement on it. CHECK asserts the cloned mark, which needs the
// returned value to have been typed as the concrete conformer.

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
