//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=A DescriptorHandleType `DescriptorHandle<T>` is a bindless handle that carries a heap index; the HLSL backend recovers the concrete resource by indexing the descriptor heap with it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a descriptor handle is not an ordinary resource binding
// but an index-carrying handle: the two handle fields live in a
// constant buffer and are used directly as a texture and a sampler.
// CHECK matches the heap subscripts in the emitted HLSL, which is where
// the stored index is turned back into a concrete resource.
//
// The other text-emit back-ends each realise the same handle in their
// own bindless form, so the fan-out pins what each one does with the
// stored index. GLSL and SPIR-V declare an unbounded descriptor array
// and subscript it with the handle's index before sampling; WGSL does
// the same with an `array<texture_2d<f32>>` heap; Metal has no
// descriptor-heap indirection, so the handle is realised directly as
// the resource member of an argument buffer and the sample is issued
// off that member.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

struct Stuff
{
    DescriptorHandle<Texture2D<float4> > tex;
    DescriptorHandle<SamplerState> samp;
}

ConstantBuffer<Stuff> cb;
RWStructuredBuffer<float4> outBuf;

//CHECK-DAG: ResourceDescriptorHeap[
//CHECK-DAG: SamplerDescriptorHeap[

//GLSL-DAG: uniform texture2D  _slang_resource_heap_{{[0-9]+}}[]
//GLSL-DAG: uniform sampler  _slang_resource_heap_{{[0-9]+}}[]
//GLSL: sampler2D(_slang_resource_heap_{{[0-9]+}}[cb_0.tex_0.x],_slang_resource_heap_{{[0-9]+}}[cb_0.samp_0.x])

//METAL-DAG: texture2d<float, access::sample> tex_0;
//METAL-DAG: sampler samp_0;
//METAL: (cb_{{[0-9]+}}->tex_0).sample((cb_{{[0-9]+}}->samp_0)

//SPIRV-DAG: OpCapability RuntimeDescriptorArray
//SPIRV-DAG: %_runtimearr_{{[0-9]+}} = OpTypeRuntimeArray %{{[0-9]+}}
//SPIRV-DAG: OpAccessChain %_ptr_UniformConstant_{{[0-9]+}} %__slang_resource_heap %{{[0-9]+}}
//SPIRV-DAG: OpAccessChain %_ptr_UniformConstant_{{[0-9]+}} %__slang_resource_heap_0 %{{[0-9]+}}

//WGSL-DAG: var _slang_resource_heap_{{[0-9]+}} : array<texture_2d<f32>>
//WGSL-DAG: var _slang_resource_heap_{{[0-9]+}} : array<sampler>
//WGSL: textureSampleLevel((_slang_resource_heap_{{[0-9]+}}[cb_0.tex_0.x]), (_slang_resource_heap_{{[0-9]+}}[cb_0.samp_0.x])
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = cb.tex.SampleLevel(cb.samp, float2(0.5, 0.5), 0.0);
}
