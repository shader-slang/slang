// The `.Handle` construction spelling of a non-uniform descriptor-heap index: a `Texture2D.Handle`
// built from `uint2(NonUniformResourceIndex(tid.x), 0)` is implicitly converted back to
// `Texture2D`, and the peephole fold that collapses this untyped-handle wrap/unwrap round-trip
// must keep the non-uniform marker on the emerging scalar heap index — the emitted code must mark
// the heap access non-uniform the same way the direct subscript spelling does (see
// `desc-heap-nonuniform.slang`), instead of leaving the marker nested inside the `uint2` /
// `uvec2` construction. The resource and the sampler heap are both covered, because the two heap
// types load through distinct descriptor-load intrinsics, so a marker-dropping regression in one
// chain must not be masked by the other still passing. SPIR-V decoration for this spelling is
// covered by `desc-heap-nonuniform-spirv.slang`.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), 0) };
    Texture2D tex = th;
    SamplerState.Handle sh = { uint2(NonUniformResourceIndex(tid.y), 0) };
    SamplerState s = sh;
    outputBuffer[0] = tex.SampleLevel(s, float2(0.0, 0.0), 0.0).x;
}

// Both heap indices must be wrapped in `NonUniformResourceIndex` directly inside the subscript
// brackets, not nested inside the `uint2` construction.
// HLSL: ResourceDescriptorHeap[NonUniformResourceIndex(
// HLSL: SamplerDescriptorHeap[NonUniformResourceIndex(

// The DXIL directive routes the emitted HLSL through DXC: reaching a valid DXIL module confirms
// DXC accepts the handle-construction path's SM6.6 heap output.
// DXIL: define void @computeMain(

// GLSL expresses the same property with `nonuniformEXT` directly on each heap index. Emission
// folds the texture and the sampler into one combined `sampler2D(texture, sampler)` constructor,
// so the resource-heap index is pinned inside `sampler2D(` and the sampler-heap index after its
// comma; either heap may carry the `_0` suffix depending on emission order, so the heap names
// are wildcards.
// GLSL: #extension GL_EXT_nonuniform_qualifier : require
// GLSL: sampler2D(_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(
// GLSL: ,_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(
