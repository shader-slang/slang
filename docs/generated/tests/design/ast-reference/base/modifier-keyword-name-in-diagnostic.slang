//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#modifier-syntaxnode
//META: doc_section_digest=ce8e569744478afff097f76c56095e0d6452e8ce6193cb8b0f043fa21c0270e5
//META: purpose=A Modifier records the keyword or attribute name that introduced it, and that recorded spelling is what an unknown-attribute diagnostic quotes back.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the modifier keeps the spelling it was introduced with
// rather than being reduced to an anonymous node. An attribute name no
// keyword table knows is written on a function; CHECK asserts the
// diagnostic quotes that exact spelling back, so the name must have
// been stored on the modifier. The diagnostic is a warning, so the
// declaration it adorns still checks.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[NoSuchAttributeXyz]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^ unknown attribute
 ^^^^^^^^^^^^^^^^^^ unknown attribute 'NoSuchAttributeXyz'
*/
// CHECK: E31000
void g() {}

void main() { g(); }
