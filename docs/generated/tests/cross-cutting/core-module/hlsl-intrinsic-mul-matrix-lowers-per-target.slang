//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=The HLSL meta-module's `mul(matrix, vector)` intrinsic lowers to per-target matrix-vector multiplication spellings.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `mul` is documented as one of the HLSL meta-module intrinsics. The
// matrix-vector form lowers to HLSL `mul`, GLSL `*` operator on
// `mat`/`vec`, and SPIR-V `OpMatrixTimesVector`. We exercise each.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3x3 m = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    float3 v = float3(float(tid.x), float(tid.y), float(tid.z));
    outBuf[tid.x] = mul(m, v);
}

// HLSL: mul(
// GLSL: {{(\*|matrixCompMult|mul)}}
// SPIRV: OpVectorTimesMatrix
