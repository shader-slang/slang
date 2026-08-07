//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Decl checking rejects an unsized member that is not the last member of a struct (E30070).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Layout-level decl boundary: an unsized array member must be the
// last member of its enclosing composite.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int data[]; int x; }
//CHECK:       ^^^^ unsized member must be last
//CHECK:       ^^^^ member with unknown size at compile time can only appear as the last member in a composite type.
// CHECK: E30070

void main() {}
