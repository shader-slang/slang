//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
//META: purpose=Inheritance checking rejects a struct that names itself as its base ("cyclic reference in inheritance graph").
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Inheritance-checking boundary: self-inheritance is a degenerate
// cycle and is reported via the cyclic-reference-in-inheritance
// diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A : A {}
/*CHECK:
           ^ support for inheritance is unstable and will be removed in future language versions, consider using composition instead
       ^ cyclic reference in inheritance graph 'A'
*/

void main() {}
