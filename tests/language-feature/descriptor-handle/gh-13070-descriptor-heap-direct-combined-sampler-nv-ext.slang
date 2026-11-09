// Under spvBindlessTextureNV + spvDescriptorHeapEXT, a combined texture-sampler cannot be recovered
// directly from a single `ResourceDescriptorHeap[i]` index: one index cannot carry the separate sampler
// index the combined-sampler load needs, so the conversion is diagnosed. The message must NOT recommend
// the `.Handle(uint2(...))` form -- under `spvBindlessTextureNV` that is itself a native handle and would
// reinterpret the `uint2` as a `uint64`. A plain texture or sampler recovered from a heap index stays
// valid and undiagnosed. See shader-slang/slang#13070.

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
    // load from the EXT heaps with NO diagnostic.
    Texture2D<float> tex = ResourceDescriptorHeap[tid.x];
    SamplerState smp = SamplerDescriptorHeap[tid.x];

    outputBuffer[0] = tex.SampleLevel(smp, float2(0, 0), 0) + combined.SampleLevel(float2(0, 0), 0).x;
}
