//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=81a3df95eff9564dff8632395b68653e53cef37e32ae6b5527c69075121fe6ab
//META: purpose=Fires diagnostic E30201 (function-redefinition) - function '~function' already has a body
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30201
//META: catalog_name=function-redefinition
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A function may be declared many times but defined once. A second body for the
// same signature is a redefinition, which the CHECK pins by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void f() {}
void f() {}
void main() { f(); }

//CHECK: E30201
