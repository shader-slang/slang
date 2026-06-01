//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W40010 (float-literal-too-small) - floating-point literal too small
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40010
//META: catalog_name=float-literal-too-small
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-zero float literal smaller than the smallest
// representable subnormal of the target type triggers W40010.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const half g = 1.0e-30h;

// CHECK: W40010
