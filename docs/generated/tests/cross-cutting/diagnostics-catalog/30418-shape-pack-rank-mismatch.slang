//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Drop placeholder for E30418 (shape-pack-rank-mismatch); see README.md
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30418
//META: catalog_name=shape-pack-rank-mismatch
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// E30418 fires for __shape* intrinsics that are not user-callable. Asserts a
// trivial program does not spuriously emit E30418. Documented in README.md.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main() {}

// CHECK-NOT: E30418
