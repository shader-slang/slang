//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=9e789f76018ea954bfb264040a694991a83baab0f112111a9b519188a6174ec6
//META: purpose=The core module's vector and matrix types are emitted in each target's own spelling: float3/float4x4 on HLSL, vec3/mat3x3 on GLSL, vec3<f32>/mat3x3<f32> on WGSL, float3/matrix<float,..> on Metal, float3/makeMatrix on CUDA and Vector/Matrix templates on C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the scalar/vector/matrix vocabulary declared by the core
// module is target-neutral in the source and target-shaped in the
// output. One shader builds a float3 and a float3x3 from a dispatch
// id (so nothing folds away) and stores an element; each prefix
// CHECKs the vector and matrix spelling that back end emits, which is
// what proves the same core-module declaration is being re-spelled
// rather than passed through verbatim.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 a = float3(float(tid.x), 1.0, 2.0);
    float3 b = a + float3(1.0, 1.0, 1.0);
    float3x3 m = float3x3(b.x, 0.0, 0.0, 0.0, b.y, 0.0, 0.0, 0.0, b.z);
    outBuf[tid.x] = m[1][1] + b.z;
}

// HLSL-DAG: float3(
// HLSL-DAG: float3x3(
// GLSL-DAG: vec3(
// GLSL-DAG: mat3x3(
// SPIRV-DAG: OpTypeVector %float 3
// METAL-DAG: float3(
// METAL-DAG: matrix<float
// WGSL-DAG: vec3<f32>(
// WGSL-DAG: mat3x3<f32>(
// CUDA-DAG: make_float3
// CUDA-DAG: makeMatrix<float, 3, 3>
// CPP-DAG: Vector<float, 3>
// CPP-DAG: Matrix<float, 3, 3>
