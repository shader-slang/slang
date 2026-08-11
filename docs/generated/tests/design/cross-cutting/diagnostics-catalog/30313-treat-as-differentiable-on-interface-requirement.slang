//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=3d3cfaab0521dad30fde00c21e4e2830740609e269e110699f6b04543ae3ca8d
//META: purpose=Fires diagnostic E30313 (treat-as-differentiable-on-interface-requirement) - [TreatAsDifferentiable] cannot be applied to interface requirement
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30313
//META: catalog_name=treat-as-differentiable-on-interface-requirement
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[TreatAsDifferentiable]` on an interface requirement is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IDoer
{
    [TreatAsDifferentiable]
    float doit(float x);
}

void main() {}

//CHECK: 30313
