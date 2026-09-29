//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=1eb15a8f53a6d50fc104f719b93c302a65314fe840e8cbd0c9f9d6c59793afef
//META: purpose=Fires diagnostic E20002 (syntax-error) - syntax error
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20002
//META: catalog_name=syntax-error
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An initializer must contain an expression. Omitting it leaves the parser with
// no expression to build, and the CHECK pins the generic syntax-error code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = ; }

//CHECK: E20002
