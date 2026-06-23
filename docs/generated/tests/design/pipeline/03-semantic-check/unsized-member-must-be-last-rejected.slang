//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
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
