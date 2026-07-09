//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=602dd96dd9a5139f2ccd4e3f857107fe47c9705a3ba52eae7e88a90893f9d664
//META: purpose=RWTexture2D<float4> emits as a global var with WGSL texture_storage_2d<...,read_write> type and textureStore/textureLoad lowerings.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWTexture2D<float4> maps to WGSL's storage-texture family:
// `texture_storage_2d<rgba32float, read_write>` (the format is
// inferred from the Slang element type, and the access mode is
// explicit). Subscript-store lowers to `textureStore(...)` and
// subscript-load lowers to `textureLoad(...)`.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWTexture2D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid.xy] = float4(1.0);
    outBuf[tid.x] = tex[tid.xy];
}

// CHECK: texture_storage_2d<
// CHECK: read_write
// CHECK: textureStore
// CHECK: textureLoad
