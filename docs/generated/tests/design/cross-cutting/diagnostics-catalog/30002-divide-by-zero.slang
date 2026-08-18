//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f94e34150ae422d4db154f28a88964f57b87c0d0fecc4cb46b2b116b1d5e71a6
//META: purpose=Fires diagnostic E30002 (divide-by-zero) - divide by zero
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30002
//META: catalog_name=divide-by-zero
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Constant folding evaluates integer division at compile time, so a literal
// zero divisor is caught before codegen. The CHECK pins the divide-by-zero
// error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main() { int x = 1 / 0; }

//CHECK: E30002
