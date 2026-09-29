//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#inheritancedecl
//META: doc_section_digest=faee42cf53078ee6cd6b1f69e2f5e33966ac4159fe9a593a801ecde8862f2dc6
//META: purpose=An InterfaceDecl declares an interface and an inheritance clause ties a conforming type to it; constrained dispatch picks the conformer's implementation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `: IShape` clause on a struct records the conformance that
// generic dispatch relies on. The generic calls the interface's method on its
// parameter and the CHECK asserts the struct's own implementation ran, which
// requires the inheritance clause's witness to have been resolved.

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
