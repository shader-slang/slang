//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=81571315b745d6c86e849e4932c9fea3e9674c90a4abf302edfa6e27c8367741
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

//CHECK: 30311
