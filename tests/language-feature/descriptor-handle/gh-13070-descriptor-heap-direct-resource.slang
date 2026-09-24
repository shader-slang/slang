// Companion to gh-13070-bindless-nv-descriptor-heap-coexist.slang. With both spvBindlessTextureNV and
// spvDescriptorHeapEXT enabled, a resource obtained directly from an explicit heap index
// (`Texture2D t = ResourceDescriptorHeap[i]`, `SamplerState s = SamplerDescriptorHeap[j]`) must load
// from the EXT descriptor heap: the index is not a native bindless handle and must not be reinterpreted
// as one. A directly-bound native `.Handle` uniform in the same module still uses the native NV
// conversion, so the two representations coexist. See shader-slang/slang#13070.
//
// The shader has exactly one native texture uniform, one native sampler uniform, one explicit-index
// texture, and one explicit-index sampler. So there must be exactly one native image conversion and one
// native sampler conversion (the two uniforms) plus one untyped access chain into each EXT heap (the two
// explicit indices). The exact-count runs below pin this: a *second* native conversion would mean an
// explicit heap index wrongly entered the native-conversion path instead of loading from the EXT heap.

//TEST:SIMPLE(filecheck=CHECK):  -target spirv-asm -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=IMGCNT): -target spirv-asm -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SMPCNT): -target spirv-asm -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage compute -entry computeMain

[[vk::binding(0)]] RWStructuredBuffer<float> outputBuffer;

// Directly-bound native bindless handles — stay native.
uniform Texture2D<float>.Handle nativeTex;
uniform SamplerState.Handle nativeSampler;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;

    // Explicit heap indices -> load from the EXT resource/sampler heaps.
    Texture2D<float> t = ResourceDescriptorHeap[i];
    SamplerState s = SamplerDescriptorHeap[i];
    float fromHeap = t.SampleLevel(s, float2(0, 0), 0);

    // Native handles -> native NV conversion, in the same module.
    float fromNative = nativeTex.SampleLevel(nativeSampler, float2(0, 0), 0);

    outputBuffer[0] = fromHeap + fromNative;
}

// The explicit-index texture and sampler load from the EXT heaps (untyped access chains into the
// variables decorated BuiltIn ResourceHeapEXT / SamplerHeapEXT), and the directly-bound native handles
// still use the NV conversions:
//CHECK-DAG: OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
//CHECK-DAG: OpDecorate %slang_samplerHeap BuiltIn SamplerHeapEXT
//CHECK-DAG: OpUntypedAccessChainKHR {{.*}}%slang_resourceHeap
//CHECK-DAG: OpUntypedAccessChainKHR {{.*}}%slang_samplerHeap
//CHECK-DAG: OpConvertUToImageNV
//CHECK-DAG: OpConvertUToSamplerNV

// Exactly one native image conversion (the native texture uniform); the explicit-index texture does not
// also take the native path.
//IMGCNT-COUNT-1: OpConvertUToImageNV
//IMGCNT-NOT: OpConvertUToImageNV

// Exactly one native sampler conversion (the native sampler uniform); the explicit-index sampler does
// not also take the native path.
//SMPCNT-COUNT-1: OpConvertUToSamplerNV
//SMPCNT-NOT: OpConvertUToSamplerNV
