//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
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
