//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=1cac63002ca6d2a65fe6ef33e66316f967a340590151c6c74af40b1cab75c97b
//META: purpose=Fires diagnostic E40018 (overload-candidate-argument-type-mismatch) - argument ~argIndex:Int does not match: expected '~expectedType:Type', got '~actualType:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40018
//META: catalog_name=overload-candidate-argument-type-mismatch
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When no overload applies, each rejected candidate is explained by a note that
// names the first argument position that does not match. Using a type name where
// a value is expected makes every `operator+` candidate fail on argument 0, and
// the CHECK pins that per-candidate note's code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct S { int a; }
void main() { int y = S + 1; }

//CHECK: E40018
