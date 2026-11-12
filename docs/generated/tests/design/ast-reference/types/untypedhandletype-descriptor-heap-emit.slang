//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-02T10:45:10+00:00
//META: source_commit=6815ed82d5e8c95b0e1258ca2eba0ca9c7825d86
//META: doc_ref=docs/generated/design/ast-reference/types.md#untypedresourcehandletype-and-untypedsamplerhandletype
//META: doc_section_digest=f268b5de37f7e6c9147bb9540c9ab7e8d8517dbcdbc595542a45dcdfef731730
//META: purpose=Subscripting ResourceDescriptorHeap and SamplerDescriptorHeap yields untyped handles whose concrete resource type is recovered from the conversion target, and neither handle type survives to code emission - each target emits a heap indexed by the uint index instead.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two descriptor-heap handle types end to end. The heap
// subscripts are written with no type annotation of their own; the concrete
// `Texture2D` and `SamplerState` types come from the declared targets of the
// two initializations, which is the "recovered from the target of the implicit
// conversion" half of the claim — the resource and sampler families stay
// disjoint because each conversion is declared separately. The CHECKs then
// observe that no handle type reaches the output: HLSL re-spells the two heap
// subscripts directly, GLSL and WGSL emit a runtime-sized heap array indexed
// by the uint, and SPIR-V emits the `RuntimeDescriptorArray` capability with a
// heap variable. The index feeding the subscript is a uniform so it cannot be
// folded away before emission.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;
uniform uint idx;

[numthreads(1, 1, 1)]
void computeMain()
{
    Texture2D t = ResourceDescriptorHeap[idx];
    SamplerState s = SamplerDescriptorHeap[idx];
    outputBuffer[0] = t.SampleLevel(s, float2(0, 0), 0).x;
}

// HLSL keeps the SM6.6 heap spelling and converts each handle to the
// concrete resource type at the use site.
//HLSL-DAG: Texture2D<float4 {{.*}}> {{[A-Za-z0-9_]+}} = Texture2D<float4 {{.*}}>(ResourceDescriptorHeap[
//HLSL-DAG: SamplerState {{[A-Za-z0-9_]+}} = SamplerState(SamplerDescriptorHeap[

// SPIR-V turns the heap into a runtime descriptor array; the handle itself
// is gone, leaving only the index-into-heap access chain.
//SPV-DAG: OpCapability RuntimeDescriptorArray
//SPV-DAG: OpName %{{[A-Za-z0-9_]+}} "__slang_resource_heap"

// GLSL and WGSL both emit an unsized heap array subscripted by the index.
//GLSL-DAG: uniform texture2D {{.*}}_slang_resource_heap{{[A-Za-z0-9_]*}}[];
//GLSL-DAG: uniform sampler {{.*}}_slang_resource_heap{{[A-Za-z0-9_]*}}[];
//WGSL-DAG: _slang_resource_heap{{[A-Za-z0-9_]*}} : binding_array<texture_2d<f32>>;
//WGSL-DAG: _slang_resource_heap{{[A-Za-z0-9_]*}} : binding_array<sampler>;
