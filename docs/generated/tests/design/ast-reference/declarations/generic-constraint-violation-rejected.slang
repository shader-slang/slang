//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
//META: purpose=A subtype constraint is checked at the specialization site; a type argument that does not conform is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative half of the subtype-constraint claim. A generic
// constrained on an interface is explicitly specialized with a struct that
// does not conform; the CHECKs pin the conformance diagnostic at the type
// argument, so the constraint is enforced rather than assumed.

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
