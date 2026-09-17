// Direct-resource conversion of a combined texture-sampler from a single `ResourceDescriptorHeap[i]`
// index is rejected on the targets whose combined-sampler load consumes a separate sampler index (HLSL,
// SPIR-V `spvDescriptorHeapEXT`): one index cannot specify both the texture and the sampler. This pins
// the NV+EXT case and, in particular, that the remedy does NOT point at `.Handle(uint2(...))` — under
// `spvBindlessTextureNV` that form is itself a native handle and would reinterpret the `uint2` as a
// `uint64` (the very shader-slang/slang#13070 miscompile). The single-index texture and sampler
// conversions on the same line must still load from the EXT heap without a diagnostic, so an
// over-broad guard (rejecting every encodable direct-resource conversion) would fail this test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage compute -entry computeMain

[[vk::binding(0)]] RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Sampler2D combined = ResourceDescriptorHeap[tid.x];
//CHECK:                                       ^ static assertion failed
//CHECK:                                       ^ static assertion failed, Cannot initialize a combined texture-sampler from a single ResourceDescriptorHeap index; load the texture and sampler separately from ResourceDescriptorHeap and SamplerDescriptorHeap.

    // Regression guard: a plain texture and a plain sampler recovered directly from heap indices must
    // load from the EXT heaps with NO diagnostic (the guard must not reject every encodable conversion).
    Texture2D<float> tex = ResourceDescriptorHeap[tid.x];
    SamplerState smp = SamplerDescriptorHeap[tid.x];

    outputBuffer[0] = tex.SampleLevel(smp, float2(0, 0), 0) + combined.SampleLevel(float2(0, 0), 0).x;
}
