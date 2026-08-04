//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
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
