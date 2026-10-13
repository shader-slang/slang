//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#untypedresourcehandletype-and-untypedsamplerhandletype
//META: doc_section_digest=23856356bace576e711fa2b6686659e33aa5c5f0a6c6c1955ee17350d49ad6ca
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
//WGSL-DAG: _slang_resource_heap{{[A-Za-z0-9_]*}} : array<texture_2d<f32>>;
//WGSL-DAG: _slang_resource_heap{{[A-Za-z0-9_]*}} : array<sampler>;
