//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=cb681a5a5428e160c2311cbf091e8eda34ecba068f868b779a464083304bb62f
//META: purpose=A `<T : I>` parameter bound records a subtype constraint; a satisfying type argument lets the body call the interface's members.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a constraint written in the generic parameter list admits the
// interface's members inside the body. The generic calls `area()` on its
// parameter and is instantiated with a conforming struct; the CHECK asserts
// the conformer's result, which is only reachable through the constraint.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int area(); }

struct Square : IShape
{
    int s;
    int area() { return s * s; }
}

int polyArea<T : IShape>(T shape) { return shape.area(); }

void main()
{
    Square sq; sq.s = 4;
    //CHECK: 16
    printf("%d\n", polyArea(sq));
}
