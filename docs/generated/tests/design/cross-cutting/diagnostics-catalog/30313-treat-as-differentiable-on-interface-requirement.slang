//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: 30313
