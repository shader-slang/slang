//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=050edfb9754fda57841049434af6fd653124ac1cafaaa287b699d7cc2cbeb911
//META: purpose=Fires diagnostic E40009 (float-literal-unrepresentable) - floating-point literal unrepresentable
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40009
//META: catalog_name=float-literal-unrepresentable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A float literal that exceeds the max representable value of
// the target type (here `half`) is converted with E40009.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const half g = 1.0e10h;

//CHECK: E40009