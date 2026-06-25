//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-06-24T09:20:31Z
//META: source_commit=b25e06aeb8862d207541c3d812c57cb9e381b802
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A GenericTypeConstraintDecl is enforced at the call site; specializing with a type that does not satisfy the constraint is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative form of the where-clause constraint role: a
// constrained generic cannot be specialized at a type that does not
// satisfy the constraint, and the checker diagnoses the call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IShape { int area(); }

struct NotAShape { int v; }

int polyArea<T : IShape>(T shape) { return shape.area(); }
/*CHECK:
    ^^^^^^^^ see declaration of func polyArea<T> -> int
*/

void main()
{
    NotAShape n; n.v = 1;
    int x = polyArea(n);
//CHECK:            ^ type argument 'NotAShape' does not conform to the required interface 'IShape'
}
