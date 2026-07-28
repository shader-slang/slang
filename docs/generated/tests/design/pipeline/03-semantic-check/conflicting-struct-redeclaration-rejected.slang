//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
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
// CHECK: E30200

void main() {}
