//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=Core-module typedef `float32_t` resolves and lowers to a 32-bit float spelling on each text-emit backend.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The core module declares `typedef float float32_t;`. We use the
// alias in a buffer parameter and FileCheck that each backend lowers
// it to the 32-bit float spelling that backend uses.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<float32_t> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float32_t v = float32_t(tid.x) + 1.0f;
    outBuf[tid.x] = v;
}

// HLSL: float
// GLSL: float
// SPIRV: OpTypeFloat 32
// CUDA: float
