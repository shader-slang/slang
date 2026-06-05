//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W40009 (float-literal-unrepresentable) - floating-point literal unrepresentable
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40009
//META: catalog_name=float-literal-unrepresentable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A float literal that exceeds the max representable value of
// the target type (here `half`) is converted with W40009.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const half g = 1.0e10h;

// CHECK: W40009
