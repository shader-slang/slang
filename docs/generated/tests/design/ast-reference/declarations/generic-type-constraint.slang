//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
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
