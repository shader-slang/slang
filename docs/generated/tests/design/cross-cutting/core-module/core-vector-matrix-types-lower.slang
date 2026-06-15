//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=The core module declares the scalar/vector/matrix type vocabulary; `float3` and `float3x3` are usable without import and lower per target.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists "scalar / vector / matrix types" as part of the
// core module's declarations. We use `float3` and `float3x3` to
// observe each backend's spelling — HLSL `float3`/`float3x3`,
// GLSL `vec3`/`mat3`, SPIR-V `OpTypeVector`/`OpTypeMatrix`.

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

// HLSL: float3
// GLSL: {{(vec3|mat3)}}
// SPIRV: OpTypeVector %float 3
// SPIRV: OpTypeMatrix
