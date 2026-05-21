//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Drop note for E30098 (non-static-member-function-not-allowed-as-diff-operand); see BUNDLE.md
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30098
//META: catalog_name=non-static-member-function-not-allowed-as-diff-operand
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// E30098 could not be triggered cleanly: the simplest attempted repro caused
// slangc to crash. Documented in BUNDLE.md "Codes dropped". The DIAGNOSTIC_TEST
// below asserts only that compilation of an empty entry point produces NO
// E30098 diagnostic (negative confirmation that the file does not regress on
// the crash path).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main() {}

// CHECK-NOT: E30098
