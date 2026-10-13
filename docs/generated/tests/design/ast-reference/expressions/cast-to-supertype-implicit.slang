//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#astypeexpr--istypeexpr--casttosupertypeexpr
//META: doc_section_digest=a1b5995779c673c26a303bd7b4c37473fda1915905c75120601e573453457f3a
//META: purpose=A CastToSuperTypeExpr is inserted when a concrete struct is passed where an interface is expected; the interface method dispatches to the concrete implementation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that passing a concrete struct where an interface parameter is
// expected inserts the up-cast that carries the conformance witness. The
// callee invokes the interface method; the CHECK asserts the concrete type's
// own implementation ran, which requires the witness to have been threaded.

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
