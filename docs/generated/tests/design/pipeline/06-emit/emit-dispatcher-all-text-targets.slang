//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#emit-dispatcher
//META: doc_section_digest=931b6be8623259835eb4a857276a2ea3e6c3d0ca8f46230a8f6b9a1c92951550
//META: purpose=One compute kernel routed through the emit dispatcher produces target-shaped text for every textual backend plus the direct SPIR-V backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a single `TargetRequest` per compile selects a different
// source emitter and that each one produces text only that backend can
// produce. The same kernel is compiled seven times; each prefix asserts a
// marker no other backend emits (HLSL `register(u0)`, GLSL `#version`,
// SPIR-V `OpEntryPoint GLCompute`, Metal `metal_stdlib`, WGSL
// `@workgroup_size`, CUDA `__global__`, C++ `SLANG_PRELUDE_EXPORT`). If the
// dispatcher routed to the wrong emitter, the marker would be absent.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL-DAG: register(u0)
// HLSL-DAG: numthreads(8, 1, 1)

// GLSL-DAG: {{^}}#version
// GLSL-DAG: local_size_x = 8

// SPIRV-DAG: OpEntryPoint GLCompute
// SPIRV-DAG: OpExecutionMode {{.*}} LocalSize 8 1 1

// METAL-DAG: metal_stdlib
// METAL-DAG: void main_

// WGSL-DAG: @compute
// WGSL-DAG: @workgroup_size(8, 1, 1)

// CUDA-DAG: extern "C" __global__ void main_
// CUDA-DAG: SLANG_globalParams

// CPP-DAG: SLANG_PRELUDE_EXPORT
// CPP-DAG: GlobalParams_
