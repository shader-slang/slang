//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=Core-module typedef `int32_t` is visible without explicit import and lowers to a 32-bit int on every text-emit backend.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The core module declares `typedef int int32_t;` in its preamble.
// Because the core module is implicitly in scope, user code can
// declare and use an `int32_t` without an `import core;`. We verify
// the typedef compiles to the expected 32-bit integer spelling on
// HLSL, GLSL, SPIR-V, and CUDA targets.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int32_t> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int32_t v = int32_t(tid.x);
    outBuf[tid.x] = v;
}

// HLSL: int
// GLSL: int
// SPIRV: OpTypeInt 32 1
// CUDA: int
