//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#inheritancedecl
//META: doc_section_digest=ba39e4dc5746ee086307f1361f8cdf3c80f1b5ff6a1f0c8ad4ac291321ad85c5
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
