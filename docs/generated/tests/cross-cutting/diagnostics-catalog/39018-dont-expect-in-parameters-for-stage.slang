//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39018 (dont-expect-in-parameters-for-stage) - stage does not support in parameters
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39018
//META: catalog_name=dont-expect-in-parameters-for-stage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A raygeneration entry point with an `in` parameter triggers
// E39018.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage raygeneration -entry main

[shader("raygeneration")]
void main(float4 x)
{
}

// CHECK: E39018
