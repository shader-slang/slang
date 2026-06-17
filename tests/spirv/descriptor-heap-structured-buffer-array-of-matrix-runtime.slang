// GPU EXECUTION regression test for #11483: reading a nested `float4x4[4]`
// struct member through a descriptor-heap (`spvDescriptorHeapEXT`) buffer pointer
// must return the correct array element, not data from a wrong offset.
//
// Background (#11483): the reporter saw a `ConstantBuffer<UniformBuffer>` fetched
// via the descriptor-heap untyped-pointer path read WRONG data specifically for a
// nested `shadow_transforms[4]` (`float4x4[4]`) member, while every other member,
// and the same struct accessed through a bound `[[vk::binding]]` ConstantBuffer,
// read correctly. The emitted SPIR-V offsets/strides were byte-identical between the
// bound and heap paths (see the compile-time layout pin in
// tests/spirv/descriptor-heap-constant-buffer-array-of-matrix.slang); only the base
// pointer differed (a heap-derived `OpBufferPointerEXT` instead of a bound
// `OpVariable`). So the defect, if present, can only be caught by GPU execution —
// a static / `spirv-val` check cannot see a wrong-data readback. This test provides
// that execution check for CI's GPU machines.
//
// Why StructuredBuffer instead of ConstantBuffer: the render-test harness can only
// place a resource into the descriptor heap when it is bound as a buffer descriptor
// (the `getDescriptorHandle` path is reached from `assignBuffer`/`assignTexture`/
// `assignSampler`, never from the `cbuffer`/object path) — so a
// `ConstantBuffer<T>.Handle` cannot be heap-bound at runtime by the test
// infrastructure, only a `StructuredBuffer<T>.Handle` can. Both exercise the same
// defect surface: a heap-derived `OpBufferPointerEXT` pointer indexed into a nested
// `float4x4[4]` member via an access chain. The ConstantBuffer-specific block layout
// remains pinned by the compile-time companion test referenced above.
//
// Each `float4x4` is filled with a single distinguishing value, so reading any
// element of `shadowTransforms[i]` yields exactly `10 + i` independent of the
// matrix's row/column-major storage — isolating the array-index arithmetic (the
// reported defect) from matrix layout. A wrong base/offset would read a neighbouring
// array slot (or a leading matrix) and produce a different value.

//TEST:COMPARE_COMPUTE(filecheck-buffer=RUNTIME): -vk -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -emit-spirv-directly

// Runtime readback: the four array elements (10..13) plus a leading-matrix and a
// trailing-member control, in output order.
// RUNTIME: type: uint
// RUNTIME-NEXT: 10
// RUNTIME-NEXT: 11
// RUNTIME-NEXT: 12
// RUNTIME-NEXT: 13
// RUNTIME-NEXT: 1
// RUNTIME-NEXT: 20

// Compile sanity: the heap handle read must lower to a descriptor-heap buffer pointer
// (the path #11483 is about), never a copy through a Function-class pointer.
// CHECK: OpCapability DescriptorHeapEXT
// CHECK: OpBufferPointerEXT

struct CameraData
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float4x4 invView;
    float4x4 invProj;
    float4x4 invViewProj;
    float4x4 shadowTransforms[4];
    float4 cascadeSplits;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

// CameraData laid out contiguously (164 floats, 656 bytes). Each matrix is uniform:
// view=1, proj=2, viewProj=3, invView=4, invProj=5, invViewProj=6 (offsets 0..384),
// shadowTransforms[0..3]={10,11,12,13} (offset 384), cascadeSplits={20,21,22,23} (640).
// Values MUST be float literals (`10.0`, not `10`): the render-test data parser stores
// a bare integer token as raw uint32 bits, which reads back as a denormal float.
// TEST_INPUT must also stay on one line — line continuation is not supported.
//TEST_INPUT: set cameraData = ubuffer(data=[1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 10.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 11.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 12.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 13.0 20.0 21.0 22.0 23.0], stride=4)
uniform StructuredBuffer<CameraData>.Handle cameraData;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Index the nested array directly off the heap-derived buffer pointer, mirroring
    // the reporter's `...Handle(...).shadow_transforms[i]` access pattern.
    output[0] = (uint)cameraData[0].shadowTransforms[0][0][0];
    output[1] = (uint)cameraData[0].shadowTransforms[1][0][0];
    output[2] = (uint)cameraData[0].shadowTransforms[2][0][0];
    output[3] = (uint)cameraData[0].shadowTransforms[3][0][0];
    // Controls: a leading matrix member and the trailing vector member read fine even
    // in the bug report — they pin that only the nested-array read is under test.
    output[4] = (uint)cameraData[0].view[0][0];
    output[5] = (uint)cameraData[0].cascadeSplits.x;
}
