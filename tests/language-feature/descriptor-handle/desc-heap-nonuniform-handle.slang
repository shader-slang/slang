// The `.Handle` construction spelling of a non-uniform descriptor-heap index: a `Texture2D.Handle`
// built from `uint2(NonUniformResourceIndex(tid.x), 0)` is implicitly converted back to
// `Texture2D`, and the peephole fold that collapses this untyped-handle wrap/unwrap round-trip
// must keep the non-uniform marker on the emerging scalar heap index — the emitted code must mark
// the heap access non-uniform the same way the direct subscript spelling does (see
// `desc-heap-nonuniform.slang`), instead of leaving the marker nested inside the `uint2` /
// `uvec2` construction. The resource and the sampler heap are both covered, because the two heap
// types load through distinct descriptor-load intrinsics, so a marker-dropping regression in one
// chain must not be masked by the other still passing. SPIR-V decoration for the resource heap
// is covered by `desc-heap-nonuniform-spirv.slang`.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
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

// The DXIL directive routes the emitted HLSL through DXC. The last `i1` operand of
// `createHandleFromHeap` is the non-uniform-index flag, so these checks pin that DXC propagates
// the marker to each heap handle — resource heap first, then sampler heap — beyond accepting the
// emitted SM6.6 heap output at all.
// DXIL: define void @computeMain(
// DXIL: @dx.op.createHandleFromHeap(i32 218, i32 %{{[0-9]+}}, i1 false, i1 true)
// DXIL: @dx.op.createHandleFromHeap(i32 218, i32 %{{[0-9]+}}, i1 true, i1 true)

// GLSL asserts the same property as `desc-heap-nonuniform.slang`, whose check block explains the
// combined `sampler2D(texture, sampler)` constructor shape these anchors pin and the heap-name
// suffix wildcards.
// GLSL: #extension GL_EXT_nonuniform_qualifier : require
// GLSL: sampler2D(_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(
// GLSL: ,_slang_resource_heap{{[0-9_]*}}[nonuniformEXT(
