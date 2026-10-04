//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b3f6e77183eaaba3b258a8005700c837efaf871cef8a0074b6692868e2f75e2a
//META: purpose=Fires diagnostic E30006 (return-needs-expression) - 'return' should have an expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30006
//META: catalog_name=return-needs-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A function with a non-void return type must return a value on every path. A
// bare `return` in an `int`-returning function omits it, and the CHECK pins
// the return-needs-expression error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
int f() { return; }
void main() { f(); }

//CHECK: E30006
