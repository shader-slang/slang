//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=61eae4cb09aaf55ca13e1b5adcdbe1bcd8d876e194e57de10e1861ffeb9396c7
//META: purpose=`hlsl.meta.slang` contributes the descriptor-heap vocabulary, and the `__subscript` on the resource-heap type turns an index into an untyped handle that converts to a concrete resource on each target in the `glsl_hlsl_spirv_wgsl` gate.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the descriptor-heap names are in the default environment
// and that indexing the heap yields a handle usable as a resource.
// `ResourceDescriptorHeap[0]` is assigned to an `RWStructuredBuffer`
// local and written through, so the handle cannot be dead-code
// eliminated. The four targets checked are exactly the ones the
// heap subscript's availability gate admits, and each CHECK pins that
// target's own lowering of the heap: HLSL re-emits the SM6.6
// `ResourceDescriptorHeap[...]` expression verbatim, while GLSL, WGSL,
// and SPIR-V lower it to the synthesized unbounded resource-heap array
// (SPIR-V additionally requiring the `RuntimeDescriptorArray`
// capability). A build in which the heap type were missing would fail
// to compile rather than mismatch a CHECK.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    RWStructuredBuffer<uint> heapBuf = ResourceDescriptorHeap[0];
    heapBuf[0] = 5;
    outputBuffer[0] = 1;
}

//HLSL: ResourceDescriptorHeap[
//GLSL: _slang_resource_heap
//SPV: OpCapability RuntimeDescriptorArray
//WGSL: _slang_resource_heap
