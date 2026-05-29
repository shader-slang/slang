//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr
//META: doc_section_digest=8acefa9f31e1283f2e07f4a5c07a19625522aed173eb000464e0d7fc4693e965
//META: purpose=A CastToSuperTypeExpr is inserted when a concrete struct is passed where an interface is expected; the interface method dispatches to the concrete implementation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says CastToSuperTypeExpr carries the conformance witness
// for an interface up-cast. We pass a concrete `Sq` where the
// parameter type is `IShape`; the checker inserts a
// CastToSuperTypeExpr and the call dispatches to Sq's `area()`.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int area(); }
struct Sq : IShape { int s; int area() { return s * s; } }

int describe(IShape shape) { return shape.area(); }

void main()
{
    Sq sq = {5};
    // CHECK: a=25
    printf("a=%d\n", describe(sq));
}
