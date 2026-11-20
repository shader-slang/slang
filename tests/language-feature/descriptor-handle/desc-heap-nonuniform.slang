// shader-slang/slang#11568: `ResourceDescriptorHeap[NonUniformResourceIndex(i)]` is a valid user
// pattern. The new untyped-handle subscript wraps the index into an untyped handle, a later
// implicit conversion unwraps it, and a peephole fold collapses that wrap/unwrap pair. This test
// pins that the `NonUniformResourceIndex` marker survives that wrap/unwrap end-to-end, so the
// emitted code still marks the heap index non-uniform regardless of opt-pass ordering.
//
// The property is asserted on the HLSL and GLSL backends, where `NonUniformResourceIndex` /
// `nonuniformEXT` are the surface forms and survival is directly observable. On both backends
// the marker must sit directly on the scalar heap index, not nested inside the `uint2` /
// `uvec2` construction that the emitted untyped-handle wrap/unwrap passes through before the
// peephole fold collapses it. The `.Handle` construction spelling, which exercises that
// wrap/unwrap in source, is covered by `desc-heap-nonuniform-handle.slang`; SPIR-V decoration
// is covered by `desc-heap-nonuniform-spirv.slang`.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D t = ResourceDescriptorHeap[NonUniformResourceIndex(tid.x)];
    SamplerState s = SamplerDescriptorHeap[NonUniformResourceIndex(tid.y)];
    outputBuffer[0] = t.SampleLevel(s, float2(0.0, 0.0), 0.0).x;
}

// Both heap indices must be wrapped in `NonUniformResourceIndex` after the untyped-handle
// wrap/unwrap and the peephole fold, directly inside the subscript brackets.
// HLSL: ResourceDescriptorHeap[NonUniformResourceIndex(
// HLSL: SamplerDescriptorHeap[NonUniformResourceIndex(

// GLSL expresses the same property with `nonuniformEXT` directly on each heap index. Emission
// folds the texture and the sampler into one combined `sampler2D(texture, sampler)` constructor,
// so the resource-heap index is pinned inside `sampler2D(` and the sampler-heap index after its
// comma; either heap may carry the `_0` suffix depending on emission order, so the heap names
// are wildcards.
// GLSL: #extension GL_EXT_nonuniform_qualifier : require
// GLSL: sampler2D(_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(
// GLSL: ,_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(

// The DXIL directive routes the emitted HLSL through DXC: reaching a valid DXIL module confirms DXC
// accepts the untyped-handle path's SM6.6 heap output.
// DXIL: define void @computeMain(
