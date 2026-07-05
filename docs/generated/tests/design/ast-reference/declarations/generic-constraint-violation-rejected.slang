//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A GenericTypeConstraintDecl is enforced at the call site; specializing with a type that does not satisfy the constraint is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative form of the where-clause constraint role: a
// constrained generic cannot be specialized at a type that does not
// satisfy the constraint, and the checker diagnoses the call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IShape { int area(); }

struct NotAShape { int v; }

int polyArea<T : IShape>(T shape) { return shape.area(); }

void main()
{
    NotAShape n; n.v = 1;
    int x = polyArea<NotAShape>(n);
//CHECK:    ^^^^^^^^ type argument doesn't conform to interface
//CHECK:    ^^^^^^^^ type argument 'NotAShape' does not conform to the required interface 'IShape'
}
