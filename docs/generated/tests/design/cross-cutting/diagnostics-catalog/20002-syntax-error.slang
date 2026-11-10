//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=17eca73f5ccd877680bd82d03d905288678f01356750ec88fc85a67096eaf76a
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
