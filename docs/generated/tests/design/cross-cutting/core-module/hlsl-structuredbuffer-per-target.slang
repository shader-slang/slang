//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=8489fad23ce3c21f6f3d24794bba24b5d2a69ab1e82e6e16bfa45871b21462f3
//META: purpose=StructuredBuffer and RWStructuredBuffer from the HLSL meta-module legalize to each target's own binding shape, and the read-only one keeps a read-only marker where the target has one.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the HLSL-named buffer types the HLSL meta-module supplies
// are real resource types that each back end binds in its own way. A
// read-only StructuredBuffer and a writable RWStructuredBuffer are
// both declared and used; every prefix CHECKs that target's binding
// syntax, and the GLSL / SPIR-V / WGSL prefixes additionally CHECK the
// read-only marker, which distinguishes the two declarations rather
// than merely proving they compiled.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

StructuredBuffer<float> inBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf[tid.x] * 2.0;
}

// HLSL-DAG: RWStructuredBuffer<float > outBuf_{{[0-9]+}} : register(u
// HLSL-DAG: StructuredBuffer<float > inBuf_{{[0-9]+}} : register(t
// GLSL-DAG: readonly buffer
// GLSL-DAG: {{^}}layout(std430, binding = {{[0-9]+}}) buffer
// SPIRV-DAG: OpDecorate %inBuf NonWritable
// SPIRV-DAG: OpDecorate %outBuf Binding
// METAL-DAG: float device* outBuf_{{[0-9]+}} {{\[\[}}buffer({{[0-9]+}}){{\]\]}}
// METAL-DAG: float device* inBuf_{{[0-9]+}} {{\[\[}}buffer({{[0-9]+}}){{\]\]}}
// WGSL-DAG: var<storage, read_write> outBuf_{{[0-9]+}}
// WGSL-DAG: var<storage, read> inBuf_{{[0-9]+}}
// CUDA-DAG: RWStructuredBuffer<float> outBuf_{{[0-9]+}}
// CUDA-DAG: StructuredBuffer<float> inBuf_{{[0-9]+}}
// CPP-DAG: RWStructuredBuffer<float> outBuf_{{[0-9]+}}
// CPP-DAG: StructuredBuffer<float> inBuf_{{[0-9]+}}
