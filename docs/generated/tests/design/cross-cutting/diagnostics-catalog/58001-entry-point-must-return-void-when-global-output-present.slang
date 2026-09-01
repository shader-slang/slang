//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7d5eb7e889dcc8f2312f59dad35c18c9e3522184415ac32842fa3fdb8710e146
//META: purpose=Fires diagnostic E58001 (entry-point-must-return-void-when-global-output-present) - entry point must return void with global outputs
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=58001
//META: catalog_name=entry-point-must-return-void-when-global-output-present
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// GLSL requires entry points to return void when a global output
// variable exists. A non-void return on the same entry triggers
// the error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -entry main -stage fragment

layout(location=0) out float4 oColor;

float4 main() : SV_TARGET {
    oColor = float4(1);
    return float4(1);
}

//CHECK: E58001
