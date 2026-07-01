//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#inheritancedecl
//META: doc_section_digest=ba39e4dc5746ee086307f1361f8cdf3c80f1b5ff6a1f0c8ad4ac291321ad85c5
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
