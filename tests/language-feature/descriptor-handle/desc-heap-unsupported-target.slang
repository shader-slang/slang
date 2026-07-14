// Negative test for shader-slang/slang#11568: on a target that cannot lower a `uint2` descriptor
// handle (here Metal), the `ResourceDescriptorHeap[i]` / `SamplerDescriptorHeap[j]` syntax must be
// DIAGNOSED at the indexing site, not silently miscompiled. Before the gate, the texture form
// reached `getDescriptorFromHandle`'s target-independent `default:` arm and emitted a raw `uint2`
// with a `.read`/`.sample` called on it. The `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]`
// on the heap subscript (and the untyped-handle constructor it calls) now rejects it up front,
// exactly as explicit `DescriptorHandle<T>(uint2)` construction is rejected on these targets.
//
// The two heaps are gated by separate subscripts (`__ResourceDescriptorHeapType` /
// `__SamplerDescriptorHeapType`), so each is exercised from its own entry point — `ResHeap`
// (resource + buffer family) and `SampHeap` (sampler family) — and asserted independently, so the
// gate cannot silently regress for one family while the other still rejects.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=RES): -target metal -stage compute -entry resourceMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SAMP): -target metal -stage compute -entry samplerMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void resourceMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D                 t   = ResourceDescriptorHeap[tid.x];
    RWStructuredBuffer<float> buf = ResourceDescriptorHeap[tid.z];
    outputBuffer[0] = t.Load(int3(0, 0, 0)).x + buf[0];
}

[numthreads(1, 1, 1)]
void samplerMain(uint3 tid : SV_DispatchThreadID)
{
    // Reference only `SamplerDescriptorHeap` so the diagnosed feature is the sampler-heap
    // subscript/conversion specifically, not the resource one.
    SamplerState s = SamplerDescriptorHeap[tid.y];
    outputBuffer[0] = 0.0;
}

// RES: E36107
// SAMP: E36107
