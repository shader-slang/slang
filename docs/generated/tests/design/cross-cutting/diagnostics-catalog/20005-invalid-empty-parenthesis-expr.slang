//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=767c78836f18de13ebb9609c02173b7e98bb12975690b266718d4031fb636ea5
//META: purpose=Fires diagnostic E20005 (invalid-empty-parenthesis-expr) - invalid empty parentheses expression
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20005
//META: catalog_name=invalid-empty-parenthesis-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `()` is not an expression: parentheses must wrap something. The CHECK pins
// the dedicated empty-parentheses error rather than a generic syntax error,
// showing the parser recognises this shape specifically.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = (); }

//CHECK: E20005
