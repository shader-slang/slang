//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=fec20f7bcac8fcdc2dfbdab25d05b3d7b296b884cc6c891f799e2e484b45a0a8
//META: purpose=Fires diagnostic E41027 (accessing-value-of-none-optional) - accessing .value on an Optional that is always none
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41027
//META: catalog_name=accessing-value-of-none-optional
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reading `.value` from an Optional that is statically `none`
// triggers the none-optional-access diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() {
    Optional<int> opt = none;
    o[0] = opt.value;
}

//CHECK: E41027
