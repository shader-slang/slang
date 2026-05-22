//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/ast-reference/base.md#modifier-syntaxnode
//META: doc_section_digest=4022b505250fb748fc38ae952be0700e371b5a3704ae0d911aa7fe1b17f68532
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
