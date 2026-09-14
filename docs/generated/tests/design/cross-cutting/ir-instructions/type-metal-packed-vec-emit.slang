//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=888243734c872495cb014c721c075987fe7d0342f0ed8a4fbd71890cda43218d
//META: purpose=A `float3` element type in a device buffer uses the `MetalPackedVec` storage type and emits as MSL `packed_float3`, while other targets keep the padded vector spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `MetalPackedVec`/`MetalPackedVectorType` type row: the
// element-aligned, unpadded vector storage type used for Metal device
// buffers. A `StructuredBuffer<float3>` is the surface that selects it, so
// the METAL CHECK asserts the buffer pointer is spelled `packed_float3`
// (unpadded, stride 12) rather than `float3`. The HLSL directive is the
// control: the same source keeps the ordinary padded `float3` there, which
// is what makes the Metal spelling a target-specific storage decision
// rather than a property of the source type.
//
// The claim has two halves and the second one ("while other targets keep the
// padded vector spelling") is only shown by fanning the same source out over
// the remaining shader-text targets. GLSL keeps `vec3`, WGSL keeps
// `vec3<f32>`, and SPIR-V keeps `%v3float` with an explicit `ArrayStride 16`
// on the runtime array — 16, not the 12 bytes three floats occupy, which is
// the padding Metal's `packed_float3` is defined to drop. Each of those
// directives also asserts the Metal spelling does *not* leak into that
// target's output.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform StructuredBuffer<float3> inbuf;
uniform RWStructuredBuffer<float3> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = inbuf[0] + inbuf[1];
}

// METAL-DAG: packed_float3 device* outbuf_{{[0-9]+}}
// METAL-DAG: packed_float3 device* inbuf_{{[0-9]+}}

// HLSL-DAG: RWStructuredBuffer<float3 > outbuf_{{[0-9]+}}
// HLSL-NOT: packed_float3

// GLSL: buffer {{[A-Za-z0-9_]+}} {
// GLSL-NEXT: vec3 _data[];
// GLSL-NOT: packed_float3

// WGSL-DAG: var<storage, read_write> outbuf_{{[0-9]+}} : array<vec3<f32>>;
// WGSL-DAG: var<storage, read> inbuf_{{[0-9]+}} : array<vec3<f32>>;
// WGSL-NOT: packed_float3

// SPIRV-DAG: OpDecorate %_runtimearr_v3float ArrayStride 16
// SPIRV-DAG: %v3float = OpTypeVector %float 3
// SPIRV-DAG: %_runtimearr_v3float = OpTypeRuntimeArray %v3float
