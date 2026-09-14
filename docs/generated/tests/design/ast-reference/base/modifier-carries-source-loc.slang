//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#modifier-syntaxnode
//META: doc_section_digest=ce8e569744478afff097f76c56095e0d6452e8ce6193cb8b0f043fa21c0270e5
//META: purpose=A Modifier is itself a syntax node with its own source location, so a misplaced attribute is diagnosed at the attribute's column rather than at the declaration it adorns.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the modifier node records where it was written.
// `[ForceInline]` is applied to a struct, which is not a valid
// placement; the caret annotation sits under the attribute on the
// attribute's own line, two lines above the struct name. CHECK pins
// E31002 and the attribute-keyword span.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[ForceInline]
/*CHECK:
 ^^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^^ attribute 'ForceInline' is not valid here
*/
// CHECK: E31002
struct S { int x; };

void main() {}
