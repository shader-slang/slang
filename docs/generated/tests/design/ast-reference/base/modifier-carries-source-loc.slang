//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#modifier-syntaxnode
//META: doc_section_digest=ce8e569744478afff097f76c56095e0d6452e8ce6193cb8b0f043fa21c0270e5
//META: purpose=Modifier derives from SyntaxNode and therefore from SyntaxNodeBase, so a Modifier carries its own SourceLoc; a misplaced attribute is diagnosed at the attribute's column, not at the host decl's column.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the Modifier node has its own source location, not
// just the location of the declaration it adorns. We apply
// `[ForceInline]` to a struct declaration — that's an invalid
// placement — and the diagnostic caret points at the attribute
// keyword `ForceInline`, not at the struct's name. That can only
// happen if the modifier node carries its own loc.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[ForceInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'ForceInline' is not valid here
*/
// CHECK: E31002
struct S { int x; };

void main() {}
