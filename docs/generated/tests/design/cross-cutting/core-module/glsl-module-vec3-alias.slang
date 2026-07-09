//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=af07acb9006b2e6bd2486c2a95f1ca55cef989b24178836276686391dc055ee3
//META: purpose=The GLSL module provides `vec3` as an alias for `float3`; using `vec3` in Slang source compiles and lowers to the same per-target spelling as `float3`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GLSL builtin module supplies `vec3` as a float3 alias
// once the session opts in to GLSL support (-allow-glsl here). We use
// `vec3` in Slang source and FileCheck that each target lowers it to
// the same spelling as `float3`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -allow-glsl
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute -allow-glsl
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute -allow-glsl

RWStructuredBuffer<vec3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    vec3 v = vec3(float(tid.x), float(tid.y), float(tid.z));
    outBuf[tid.x] = v;
}

// HLSL: float3
// GLSL: vec3
// SPIRV: OpTypeVector %float 3
