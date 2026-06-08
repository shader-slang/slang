//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Decl checking rejects a bit-field whose underlying type is not integral (E31301).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Bit-field type-check boundary: float bit-fields are not allowed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { float x : 4; };
//CHECK:         ^ bit-field type must be integral
//CHECK:         ^ bit-field type (float) must be an integral type
// CHECK: E31301

void main() { S s; }
