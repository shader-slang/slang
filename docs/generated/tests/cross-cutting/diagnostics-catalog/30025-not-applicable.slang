//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Placeholder file - code 30025 is not in bucket 1 (asserts a benign program does not spuriously emit E30025)
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30025
//META: catalog_name=not-applicable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 30025 is not in bucket 1's uncovered list. This placeholder asserts that a
// trivial well-formed program does not spuriously emit E30025.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main() {}

// CHECK-NOT: E30025
