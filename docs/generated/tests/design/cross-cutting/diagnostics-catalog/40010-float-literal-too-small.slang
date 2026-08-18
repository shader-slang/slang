//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ff94ea622e1962194e31e69e083b6850c1de29f3eed19f8502ddf0e55138535c
//META: purpose=Fires diagnostic E40010 (float-literal-too-small) - floating-point literal too small
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40010
//META: catalog_name=float-literal-too-small
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-zero float literal smaller than the smallest
// representable subnormal of the target type triggers E40010.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const half g = 1.0e-30h;

//CHECK: E40010