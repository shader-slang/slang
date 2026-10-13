//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=dd2048a7f3735502a7480d8d586af5559892eae4360094d4ce3d9920ecaa153f
//META: purpose=Fires diagnostic E30202 (function-redeclaration-with-different-return-type) - function return type mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30202
//META: catalog_name=function-redeclaration-with-different-return-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Return type is not part of the overload signature, so a later declaration of
// the same function with a different return type conflicts with the earlier
// one. The CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void f();
int f() { return 1; }
void main() { f(); }

//CHECK: E30202
