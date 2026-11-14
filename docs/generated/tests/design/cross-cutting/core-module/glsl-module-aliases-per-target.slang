//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=72709ab05fe0048310a93cb48fcbdc8ad10b25203945acdda232920d909f6344
//META: purpose=The GLSL module's vec3 and mat4 aliases name the same types as float3 and float4x4, so a shader written with GLSL-flavoured names emits each target's native vector and matrix spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GLSL module supplies aliases rather than a parallel
// type family: a shader written entirely in vec3 / vec4 / mat4 emits
// exactly the per-target spellings a float3 / float4 / float4x4
// shader would. The GLSL prefix CHECKs that vec3 stays vec3 on that
// back end while the other prefixes CHECK the target's own name, so
// an alias that had become a distinct type would show up as a
// mismatch somewhere in the fan-out.

//TEST:SIMPLE(filecheck=HLSL):-allow-glsl -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-allow-glsl -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-allow-glsl -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-allow-glsl -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-allow-glsl -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-allow-glsl -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-allow-glsl -target cpp -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    vec3 v = vec3(float(tid.x), 1.0, 2.0);
    mat4 m = mat4(float(tid.y));
    vec4 r = mul(m, vec4(v, 1.0));
    outBuf[tid.x] = r.x + r.y;
}

// HLSL-DAG: float3(
// HLSL-DAG: float4x4(
// GLSL-DAG: vec3(
// GLSL-DAG: mat4x4(
// SPIRV-DAG: OpTypeVector %float 3
// SPIRV-DAG: OpTypeMatrix %v4float 4
// METAL-DAG: float3(
// METAL-DAG: matrix<float,int(4),int(4)>
// WGSL-DAG: vec3<f32>(
// WGSL-DAG: mat4x4<f32>(
// CUDA-DAG: make_float3
// CUDA-DAG: Matrix<float, 4, 4>
// CPP-DAG: Vector<float, 3>
// CPP-DAG: Matrix<float, 4, 4>
