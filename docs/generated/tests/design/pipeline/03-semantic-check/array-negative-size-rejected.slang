//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Type checking rejects an array declared with a negative size literal (E30025).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Array-size boundary: a negative literal size is documented to be
// rejected with E30025. Verify the diagnostic text verbatim.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int arr[-3];
//CHECK: ^ array size must be non-negative
//CHECK: ^ array size must be non-negative.
// CHECK: E30025

void main() {}
