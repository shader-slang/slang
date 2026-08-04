//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Drop placeholder for E30410 (pack-query-argument-is-invalid); see README.md
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30410
//META: catalog_name=pack-query-argument-is-invalid
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// E30410 fires for pack-query intrinsics that are not user-callable. Asserts
// a trivial program does not spuriously emit E30410. Documented in README.md.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main() {}

// CHECK-NOT: E30410
