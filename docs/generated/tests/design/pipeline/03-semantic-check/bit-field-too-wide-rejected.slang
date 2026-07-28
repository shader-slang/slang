//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Decl checking rejects a bit-field whose declared width exceeds the type's width (E31300).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Bit-field boundary: a 64-bit-wide field on a 32-bit `int` exceeds
// the type width and is rejected with E31300.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int x : 64; };
//CHECK:       ^ bit-field size exceeds type width
//CHECK:       ^ bit-field size (64) exceeds the width of its type int (32)
// CHECK: E31300

void main() { S s; }
