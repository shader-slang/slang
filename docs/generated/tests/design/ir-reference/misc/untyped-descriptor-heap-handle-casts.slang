//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#untyped-descriptor-heap-handle-casts
//META: doc_section_digest=51b98bcebaf00976e5aa54073dcd0bcd459c0efa9d64525e6e8b578000d65806
//META: purpose=Indexing `ResourceDescriptorHeap` / `SamplerDescriptorHeap` wraps and unwraps the `uint` heap index through the four untyped-handle cast opcodes, none of which reach target code.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the untyped-handle story from one shader. The IR
// directive shows all four casts present after lowering, proving the handle
// really is a wrapped `uint` index rather than a distinct runtime value. The
// per-target directives then require the emitted text to name that target's
// own heap construct while asserting the cast opcode names are absent, which
// is what "these opcodes should never reach target code" means for a reader of
// the generated source.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform uint ridx;
uniform uint sidx;
Texture2D tex;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    StructuredBuffer<float> src = ResourceDescriptorHeap[ridx];
    SamplerState samp = SamplerDescriptorHeap[sidx];
    outputBuffer[0] = src[0] + tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// IR-DAG: = CastUIntToUntypedResourceHandle(%{{[A-Za-z0-9_]+}})
// IR-DAG: = CastUntypedResourceHandleToUInt(%{{[A-Za-z0-9_]+}})
// IR-DAG: = CastUIntToUntypedSamplerHandle(%{{[A-Za-z0-9_]+}})
// IR-DAG: = CastUntypedSamplerHandleToUInt(%{{[A-Za-z0-9_]+}})

//HLSL-NOT: CastUIntToUntypedResourceHandle
//HLSL-NOT: CastUntypedSamplerHandleToUInt
//HLSL-DAG: ResourceDescriptorHeap[
//HLSL-DAG: SamplerDescriptorHeap[

//GLSL-NOT: CastUIntToUntypedResourceHandle
//GLSL-NOT: CastUntypedSamplerHandleToUInt
//GLSL: _slang_resource_heap

//SPV-NOT: CastUIntToUntypedResourceHandle
//SPV-NOT: CastUntypedSamplerHandleToUInt
//SPV: OpCapability RuntimeDescriptorArray

//WGSL-NOT: CastUIntToUntypedResourceHandle
//WGSL-NOT: CastUntypedSamplerHandleToUInt
//WGSL: _slang_resource_heap
