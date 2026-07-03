//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#inheritancedecl
//META: doc_section_digest=faee42cf53078ee6cd6b1f69e2f5e33966ac4159fe9a593a801ecde8862f2dc6
//META: purpose=An InheritanceDecl carries a witness table after checking; an interface default method is reachable through the conforming type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented note that "the witness-table" recorded on
// an InheritanceDecl "is the connection point between the
// declaration AST and the witness-table machinery". A method
// declared on the interface (here, the simple `area()` requirement
// satisfied by the conforming struct) is reachable via
// interface-typed dispatch.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    int area();
}

struct Rect : IShape
{
    int w; int h;
    int area() { return w * h; }
}

int areaOf<T : IShape>(T s) { return s.area(); }

void main()
{
    Rect r;
    r.w = 4; r.h = 5;
    //CHECK: 20
    printf("%d\n", areaOf(r));
}
