// Negative test for shader-slang/slang#11568: a descriptor-heap family mismatch must be a
// compile error, not a silent runtime miscompile. Both conversion families enforce the same
// resource-vs-sampler rule, keyed off the same `__IDynamicResourceCastable<kind>` interface:
//   - Family 2 (`DescriptorHandle<T>` / `.Handle`) via the two per-kind `extension`s.
//   - Family 1 (concrete `Texture2D` / `SamplerState`) via the per-type `isSampler` split.
// So a resource-heap handle cannot reach a sampler target and vice versa, in either family.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -profile cs_6_6

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Family 2: a sampler-heap handle cannot build a resource (Texture2D) DescriptorHandle ...
    Texture2D.Handle bad1 = SamplerDescriptorHeap[tid.x];
    // ... and a resource-heap handle cannot build a sampler DescriptorHandle.
    SamplerState.Handle bad2 = ResourceDescriptorHeap[tid.y];

    // Family 1: the same rule holds for conversion straight to a concrete resource/sampler type.
    Texture2D bad3 = SamplerDescriptorHeap[tid.z];
    SamplerState bad4 = ResourceDescriptorHeap[tid.x];

    outputBuffer[0] = 0;
}

// Each mismatched assignment is rejected with E30019, naming the untyped heap handle that has no
// conversion to the requested family.
// CHECK-DAG: E30019
// CHECK-DAG: UntypedSamplerHandle
// CHECK-DAG: UntypedResourceHandle
