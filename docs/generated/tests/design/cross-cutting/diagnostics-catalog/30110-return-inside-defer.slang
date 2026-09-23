//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=1fac150ca11643ab8b2eb713bddfc41eeb7021472771829f7a74e83527e39160
//META: purpose=Fires diagnostic E30110 (return-inside-defer) - 'return' inside defer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30110
//META: catalog_name=return-inside-defer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `return` inside a `defer` body would abandon the scope exit that is already
// in progress. The CHECK pins the return-inside-defer error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { defer { return; } }

//CHECK: E30110
