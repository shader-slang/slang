//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b62e76ee69a48e993e21d32471cb78ebaabd0be669893f23eeb9fbcee6ad4105
//META: purpose=Fires diagnostic E38051 (invalid-entry-point-varying-type-for-target) - type cannot be used as entry-point varying for this target
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38051
//META: catalog_name=invalid-entry-point-varying-type-for-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `vector<bool>` as varying for SPIR-V vertex stage is rejected
// per per-target rule.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage vertex

[shader("vertex")]
float4 main(vector<bool, 4> flags : A) : SV_Position
{
    return float4(0);
}

//CHECK: E38051
