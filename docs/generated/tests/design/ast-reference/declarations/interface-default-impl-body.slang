//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=985b5affb3dd0e12342a333d794d470a82a87f6624d475545de5bdf3de72fbf2
//META: purpose=An interface member written with a body becomes a default implementation that a conformer inherits without redeclaring it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the default-implementation form of an interface member. The
// interface declares `perimeter()` with a body that calls the abstract
// `area()`; the conforming struct implements only `area()`. Calling
// `perimeter()` through a constrained generic must run the interface's body
// against the conformer's `area()`, which the CHECK's value pins down.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    int area();
    int perimeter() { return area() * 4; }
}

struct Sq : IShape
{
    int s;
    int area() { return s * s; }
}

int perimeterOf<T : IShape>(T t) { return t.perimeter(); }

void main()
{
    Sq q; q.s = 3;
    //CHECK: 36
    printf("%d\n", perimeterOf(q));
}
