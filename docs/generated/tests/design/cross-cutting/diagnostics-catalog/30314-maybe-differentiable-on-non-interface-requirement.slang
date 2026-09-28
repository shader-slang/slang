//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e6fd0a861c07a4da22586d7d950a649e7f37cfa29ec6bb55939618225c32b62d
//META: purpose=Fires diagnostic E30314 (maybe-differentiable-on-non-interface-requirement) - [MaybeDifferentiable] cannot be applied to non-interface requirement
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30314
//META: catalog_name=maybe-differentiable-on-non-interface-requirement
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// [MaybeDifferentiable] applied to a normal function (not an interface requirement)
// triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[MaybeDifferentiable]
float f(float x) { return x; }

void main() {}

//CHECK: 30314
