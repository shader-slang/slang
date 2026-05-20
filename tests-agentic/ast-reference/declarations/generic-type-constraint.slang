//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A GenericTypeConstraintDecl records a constraint (T : I); a satisfying type argument compiles and a non-satisfying one is diagnosed.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of GenericTypeConstraintDecl: the
// generic function constrains `T : IShape`. A struct that conforms
// is accepted; the dispatch reaches the conforming `area()`.

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
