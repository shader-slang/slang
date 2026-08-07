//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=22b10979ab5083a9c6d0c577210f37f6bb50a1e746c34c05d24d799e574a16a3
//META: purpose=A generic type used without specifying its arguments is rejected with a diagnostic explaining a generic argument is required.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says generic specialization is part of semantic check.
// Verify the user-visible consequence when a generic is referenced
// without arguments.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Box<T> { T x; }

void main()
{
    Box b;
/*CHECK:
    ^^^ generic type 'Box' used without argument
*/
// CHECK: E30408
}
