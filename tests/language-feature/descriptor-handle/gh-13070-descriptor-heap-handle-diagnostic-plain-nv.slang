// Under spvBindlessTextureNV alone (no spvDescriptorHeapEXT), a texture/sampler `.Handle` is still a
// native `uint64` handle, so an explicit heap index (`ResourceDescriptorHeap[i]` /
// `SamplerDescriptorHeap[j]`) cannot be stored in one. Without an EXT heap to load the descriptor from,
// the conversion is unsupported, so it is diagnosed rather than silently reinterpreting the index bits
// as a native handle. This is the plain-NV companion to `-diagnostic` (the NV+EXT case); both emit the
// same common message, which points the user at spvDescriptorHeapEXT and a direct resource/sampler
// assignment. See shader-slang/slang#13070.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain

[[vk::binding(0)]] RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D<float>.Handle h = ResourceDescriptorHeap[tid.x];
//CHECK:                                              ^ error
//CHECK:                                              ^ Cannot convert a descriptor heap index to a native bindless DescriptorHandle; with spvDescriptorHeapEXT, assign the heap entry directly to the resource or sampler type instead
    SamplerState.Handle sh = SamplerDescriptorHeap[tid.x];
//CHECK:                                          ^ error
//CHECK:                                          ^ Cannot convert a descriptor heap index to a native bindless DescriptorHandle; with spvDescriptorHeapEXT, assign the heap entry directly to the resource or sampler type instead
    Texture2D<float> t = h;
    outputBuffer[0] = t.SampleLevel(sh, float2(0, 0), 0);
}
