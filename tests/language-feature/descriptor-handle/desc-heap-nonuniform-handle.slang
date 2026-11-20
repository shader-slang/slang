// The `.Handle` construction spelling of a non-uniform descriptor-heap index: a `Texture2D.Handle`
// built from `uint2(NonUniformResourceIndex(tid.x), 0)` is implicitly converted back to
// `Texture2D`, and the peephole fold that collapses this handle representation round-trip must
// keep the non-uniform marker on the emerging scalar heap index — the emitted code must mark
// the heap access non-uniform the same way the direct subscript spelling does (see
// `desc-heap-nonuniform.slang`), instead of leaving the marker nested inside the `uint2` /
// `uvec2` construction. SPIR-V decoration for this spelling is covered by
// `desc-heap-nonuniform-spirv.slang`.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;
SamplerState samp;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), 0) };
    Texture2D tex = th;
    outputBuffer[0] = tex.SampleLevel(samp, float2(0.0, 0.0), 0.0).x;
}

// The heap index must be wrapped in `NonUniformResourceIndex` directly inside the subscript
// brackets, not nested inside the `uint2` construction.
// HLSL: ResourceDescriptorHeap[NonUniformResourceIndex(

// GLSL expresses the same property with `nonuniformEXT` directly on the heap index; the heap
// name is a wildcard so it does not depend on emission-order suffixes.
// GLSL: #extension GL_EXT_nonuniform_qualifier : require
// GLSL: _slang_resource_heap{{[0-9_]*}}[nonuniformEXT(
