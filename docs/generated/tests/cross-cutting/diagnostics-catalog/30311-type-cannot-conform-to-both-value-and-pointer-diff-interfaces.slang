//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30311 (type-cannot-conform-to-both-value-and-pointer-diff-interfaces) - type cannot conform to both value and pointer differentiation interfaces
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30311
//META: catalog_name=type-cannot-conform-to-both-value-and-pointer-diff-interfaces
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A type cannot conform to both IDifferentiable and IDifferentiablePtrType.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S : IDifferentiable, IDifferentiablePtrType {}

void main() {}

// CHECK: 30311
