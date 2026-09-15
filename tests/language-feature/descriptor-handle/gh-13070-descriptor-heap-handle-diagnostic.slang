// Under spvBindlessTextureNV a texture/sampler `.Handle` is a native `uint64` handle, so an explicit
// heap index (`ResourceDescriptorHeap[i]` / `SamplerDescriptorHeap[j]`) cannot be stored in one and the
// conversion is diagnosed rather than reinterpreting the index bits. One common message covers both
// configurations; this file covers the NV+EXT case, where the remedy is to assign the heap entry
// directly to the resource/sampler type (which loads from the EXT heap). The plain-NV variant is in the
// companion `-plain-nv` test and emits the same message. See shader-slang/slang#13070.
//
// The `combined` case additionally pins single-diagnostic behavior: a combined texture-sampler `.Handle`
// from a single index is subject to two independent restrictions (a native handle cannot hold an index;
// a combined sampler needs two indices on spvDescriptorHeapEXT), but under NV+EXT it is a native handle,
// so ONLY the native-handle message may fire — the combined-sampler message would be a contradictory
// second diagnostic. Because this test runs in exhaustive mode, a second unannotated diagnostic here
// would fail it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage compute -entry computeMain

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
    Sampler2D.Handle combined = ResourceDescriptorHeap[tid.x];
//CHECK:                                              ^ error
//CHECK:                                              ^ Cannot convert a descriptor heap index to a native bindless DescriptorHandle; with spvDescriptorHeapEXT, assign the heap entry directly to the resource or sampler type instead
    Texture2D<float> t = h;
    outputBuffer[0] = t.SampleLevel(sh, float2(0, 0), 0) + float(((uint2)combined).x);
}
