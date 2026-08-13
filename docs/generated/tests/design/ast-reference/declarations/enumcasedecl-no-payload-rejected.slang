//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=c018493cff4ea0d125fd96032328c7a624df200e9019bef0883c86de143f6642
//META: purpose=An enum case carries a tag expression and value, not a variant payload; a payload-style case is a parse error.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that enum cases are tag-carrying, not payload-carrying. Writing a
// case with a parenthesized payload type is rejected during parsing; the
// CHECK pins the unexpected-token error (E20001) at the case, which is the
// user-visible consequence of a case having no payload slot.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

enum E { A(int) }
//CHECK: E20001

void main() {}
