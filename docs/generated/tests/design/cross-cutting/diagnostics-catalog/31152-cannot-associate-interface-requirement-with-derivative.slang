//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f7a86409cb8f2cdd854dcb623096563b5ef14f2b60213f9b98a29cb13a59fec9
//META: purpose=Fires diagnostic E31152 (cannot-associate-interface-requirement-with-derivative) -- interface requirement cannot have derivative
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31152
//META: catalog_name=cannot-associate-interface-requirement-with-derivative
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface I {
    [BackwardDerivative(deriv)]
    float f(float x);
}


//CHECK: E31152
