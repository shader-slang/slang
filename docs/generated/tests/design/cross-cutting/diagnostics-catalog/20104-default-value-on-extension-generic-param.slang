//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c64c3a50d6b73dda6d0e3fbe5cd2fec284dd5b10ef2e7bc9c2d7df4cb2bbf1ef
//META: purpose=Fires diagnostic E20104 (default-value-on-extension-generic-param) - default value on extension generic parameter
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20104
//META: catalog_name=default-value-on-extension-generic-param
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Generic parameters on an `extension` are deduced from the extended type, so a
// default value for one can never be used. The CHECK pins the error that
// rejects the default on the extension's generic parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct S<T> { T v; }
extension<T = int> S<T> { int f() { return 0; } }
void main() {}

//CHECK: E20104
