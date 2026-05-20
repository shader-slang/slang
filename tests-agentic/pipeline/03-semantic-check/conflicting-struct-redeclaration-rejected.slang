//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Decl checking rejects two struct types declared with the same name as a conflicting declaration (E30200) at the type-decl shape.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion of the existing E30200 (redeclaration) coverage at the
// type-decl shape: the file table puts conflicting declaration of
// types in slang-check-decl.cpp. Verify the diagnostic fires when
// the same struct name is declared twice at file scope.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S { int x; }
struct S { int y; }
/*CHECK:
       ^ conflicting declaration
       ^ declaration of 'S' conflicts with existing declaration
*/

void main() {}
