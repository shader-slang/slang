//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=22b10979ab5083a9c6d0c577210f37f6bb50a1e746c34c05d24d799e574a16a3
//META: purpose=A generic applied with the wrong number of type arguments is rejected; the constraint solver's arity-mismatch inference failure surfaces as "cannot specialize generic".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Generic specialization section says the constraint solver records a
// GenericArgumentInferenceFailure whose kind distinguishes (among others) a
// generic arity mismatch, and that this becomes a focused diagnostic when the
// failed candidate is selected. `Pair` needs two type arguments; supplying one
// leaves the arity unsatisfiable, so the specialization is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Pair<T, U> { T a; U b; }

void main()
{
    Pair<int> p;
/*CHECK:
    ^^^^ wrong number of generic arguments
    ^^^^ generic 'Pair' expects 2 generic argument(s) but 1 were provided
*/
// CHECK: E30441
}
