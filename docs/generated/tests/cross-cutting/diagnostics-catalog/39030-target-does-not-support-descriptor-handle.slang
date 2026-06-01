//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39030 (target-does-not-support-descriptor-handle) - target does not support 'DescriptorHandle' types
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=39030
//META: catalog_name=target-does-not-support-descriptor-handle
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `DescriptorHandle<T>` for an HLSL profile that does not
// support `descriptor_handle` capability triggers E39030.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

DescriptorHandle<RWStructuredBuffer<int>> handle;

[numthreads(1,1,1)]
void main()
{
    handle[0] = 1;
}

// CHECK: E39030
