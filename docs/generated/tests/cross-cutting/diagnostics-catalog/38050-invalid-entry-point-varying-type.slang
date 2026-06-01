//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E38050
