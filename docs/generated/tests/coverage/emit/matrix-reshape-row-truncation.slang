//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a `(float3x3)float4x4` matrix narrowing emits a constructor of the first three rows each swizzled to `.xyz` on HLSL and GLSL.
//META: intent=characterization
//META: covers=source/slang/slang-emit-c-like.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins how a matrix-narrowing cast (4x4 -> 3x3) is emitted. Rather than a
// single MatrixReshape token, the c-like emitter constructs the smaller
// matrix from the first three rows of the source, each truncated with a
// `.xyz` swizzle: `float3x3(big[0].xyz, big[1].xyz, big[2].xyz)` on HLSL
// and the `mat3x3` equivalent on GLSL. The source matrix is a uniform so
// it is not folded; two diagonal elements are summed into outputBuffer.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

uniform float4x4 big;
RWStructuredBuffer<float> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3x3 small = (float3x3)big;
    outputBuffer[0] = small[0][0] + small[2][2] + (float)tid.x;
}

// HLSL builds the 3x3 from three .xyz-truncated rows of the source.
//HLSL: float3x3(globalParams_0.big_0[int(0)].xyz, globalParams_0.big_0[int(1)].xyz, globalParams_0.big_0[int(2)].xyz)
// GLSL builds the equivalent mat3x3 from the same truncated rows.
//GLSL: mat3x3(globalParams_0.big_0[0].xyz, globalParams_0.big_0[1].xyz, globalParams_0.big_0[2].xyz)
// Metal narrows via three .xyz-truncated rows of the reconstructed source matrix.
//METAL-DAG: [int(0)].xyz
//METAL-DAG: [int(1)].xyz
//METAL-DAG: [int(2)].xyz
// WGSL narrows the same way.
//WGSL-DAG: [i32(0)].xyz
//WGSL-DAG: [i32(1)].xyz
//WGSL-DAG: [i32(2)].xyz
