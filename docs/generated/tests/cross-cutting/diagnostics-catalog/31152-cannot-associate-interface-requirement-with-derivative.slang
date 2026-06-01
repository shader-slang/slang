//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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


// CHECK: E31152
