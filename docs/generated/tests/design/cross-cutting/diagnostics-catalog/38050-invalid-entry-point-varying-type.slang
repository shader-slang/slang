//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=666a9ded64a13606e9d3dc5af9800b22c613a31506789f80d9590770cf1145c8
//META: purpose=Fires diagnostic E38050 (invalid-entry-point-varying-type) - type cannot be used as entry-point varying parameter or return type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38050
//META: catalog_name=invalid-entry-point-varying-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Atomic<int>` is a storage-class wrapper and cannot appear as
// a varying parameter on any stage.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage vertex

[shader("vertex")]
float4 main(Atomic<int> a : A) : SV_Position
{
    return float4(0);
}

//CHECK: E38050
