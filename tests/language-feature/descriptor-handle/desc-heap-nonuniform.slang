// shader-slang/slang#11568: `ResourceDescriptorHeap[NonUniformResourceIndex(i)]` is a valid user
// pattern. The new untyped-handle subscript wraps the index into an untyped handle, a later
// implicit conversion unwraps it, and a peephole fold collapses that wrap/unwrap pair. This test
// pins that the `NonUniformResourceIndex` marker survives that wrap/unwrap end-to-end, so the
// emitted code still marks the heap index non-uniform regardless of opt-pass ordering.
//
// The property is asserted on the HLSL backend, where `NonUniformResourceIndex` is the SM6.6
// surface form and its survival is directly observable. A SPIR-V `NonUniform` decoration on
// descriptor-heap access is orthogonal to this PR: the equivalent pre-existing
// `DescriptorHandle<T>(uint2(NonUniformResourceIndex(i), 0))` construction emits no `NonUniform`
// decoration either (verified with and without `spvDescriptorHeapEXT`), so the untyped-handle
// path has nothing to preserve or regress on that target.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D t = ResourceDescriptorHeap[NonUniformResourceIndex(tid.x)];
    SamplerState s = SamplerDescriptorHeap[NonUniformResourceIndex(tid.y)];
    outputBuffer[0] = t.SampleLevel(s, float2(0.0, 0.0), 0.0).x;
}

// Both heap indices must still be wrapped in `NonUniformResourceIndex` after the untyped-handle
// wrap/unwrap and the peephole fold.
// HLSL: ResourceDescriptorHeap[{{.*}}NonUniformResourceIndex(
// HLSL: SamplerDescriptorHeap[{{.*}}NonUniformResourceIndex(

// The DXIL directive routes the emitted HLSL through DXC: reaching a valid DXIL module confirms DXC
// accepts the untyped-handle path's SM6.6 heap output.
// DXIL: define void @computeMain(
