//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E38051
