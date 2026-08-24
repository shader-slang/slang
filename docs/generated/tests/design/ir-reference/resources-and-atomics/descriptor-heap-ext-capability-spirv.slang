//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#descriptor-heaps
//META: doc_section_digest=6fb78bde656b3463d18dbb75963aabf532ca2ab334b749f2f1069ad2e3d8e548
//META: purpose=Compiled with `-capability spvDescriptorHeapEXT`, dereferencing a `DescriptorHandle<T>` takes the `spvDescriptorHeapEXT` arm, so `SPIRVResourceHeap`, `SPIRVSamplerHeap`, and `SPIRVLoadDescriptorFromHeap` are already present in the `-dump-ir` snapshot as core-module intrinsics, and the emitted SPIR-V names `slang_resourceHeap` / `slang_samplerHeap` and requests `SPV_EXT_descriptor_heap`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the claim that these two heap opcodes are core-module
// intrinsics in the IR from linking onward rather than things SPIR-V
// legalization invents, and that the SPIR-V arm is chosen by a capability
// rather than by the shader text.
//
// The shader is capability-agnostic; only the compile options differ from
// the plain-`spirv` sibling test in this bundle, which pins the
// `__slang_resource_heap` shape the same source produces without the
// capability. The IR directive is the load-bearing one: finding
// `SPIRVResourceHeap` in a `-dump-ir` snapshot is only possible if the
// opcode predates target legalization. The emission directive then pins
// the three user-visible tokens the capability arm produces — the
// extension request and the two heap variable names, which differ from the
// non-capability spelling by more than a prefix.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main -capability spvDescriptorHeapEXT

uniform DescriptorHandle<SamplerState> samp;
uniform DescriptorHandle<Texture2D<float4>> tex;
uniform RWStructuredBuffer<float4> sink;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = float2(tid.xy) * (1.0 / 8.0);
    sink[tid.x] = (*tex).SampleLevel(*samp, uv, 0.0);
}

// IR-DAG: SPIRVResourceHeap
// IR-DAG: SPIRVSamplerHeap
// IR-DAG: SPIRVLoadDescriptorFromHeap(

// SPV-DAG: OpExtension "SPV_EXT_descriptor_heap"
// SPV-DAG: OpName %slang_resourceHeap "slang_resourceHeap"
// SPV-DAG: OpName %slang_samplerHeap "slang_samplerHeap"
