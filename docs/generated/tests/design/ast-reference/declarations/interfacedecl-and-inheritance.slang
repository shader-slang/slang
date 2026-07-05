//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#inheritancedecl
//META: doc_section_digest=faee42cf53078ee6cd6b1f69e2f5e33966ac4159fe9a593a801ecde8862f2dc6
//META: purpose=An InterfaceDecl declares an interface and InheritanceDecl ties a conforming type to it; interface-typed dispatch picks the conforming type's implementation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented roles of InterfaceDecl and InheritanceDecl
// together. The struct's `: IShape` clause is parsed as an
// InheritanceDecl; the witness-table connection (resolved during
// checking) is what makes interface-constrained generic dispatch
// reach the correct `area()`.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int area(); }

struct Square : IShape
{
    int side;
    int area() { return side * side; }
}

int dispatchedArea<T : IShape>(T s) { return s.area(); }

void main()
{
    Square s; s.side = 6;
    //CHECK: 36
    printf("%d\n", dispatchedArea(s));
}
