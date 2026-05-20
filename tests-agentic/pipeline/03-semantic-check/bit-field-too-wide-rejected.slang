//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
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

void main() { S s; }
