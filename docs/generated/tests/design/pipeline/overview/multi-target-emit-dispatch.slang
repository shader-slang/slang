//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#emit
//META: doc_section_digest=244236b868f771e25853f0063ebf0ce241c2c1af2fde156ad5b70a5ed4f242f6
//META: purpose=Emit selects a dispatch path per target request, so one source produces HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++ artefacts, each in that back-end's own idiom.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the per-target dispatch really picks a different emitter
// for each target request rather than post-processing one shared text.
// The same shader is compiled seven times, and each CHECK pins a header /
// declaration idiom that only that back-end can produce -- an HLSL
// `register(u0)` binding, a GLSL `#version` line, a SPIR-V memory model,
// the Metal and CUDA/C++ prelude includes, and the WGSL storage-address-
// space declaration. A missing dispatch path would leave the
// corresponding directive with nothing to match.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOut[0] = int(tid.x) + 1;
}

// HLSL-DAG: RWStructuredBuffer<int
// HLSL-DAG: register(u0)

// GLSL-DAG: #version
// GLSL-DAG: layout(std430, binding = 0) buffer

// SPIRV-DAG: OpMemoryModel Logical GLSL450
// SPIRV-DAG: OpCapability Shader

// METAL-DAG: #include <metal_stdlib>
// METAL-DAG: using namespace metal;

// WGSL-DAG: var<storage, read_write>
// WGSL-DAG: @binding(0) @group(0)

// CUDA-DAG: slang-cuda-prelude.h
// CUDA-DAG: extern "C" __constant__

// CPP-DAG: slang-cpp-prelude.h
// CPP-DAG: SLANG_PRELUDE_NAMESPACE
