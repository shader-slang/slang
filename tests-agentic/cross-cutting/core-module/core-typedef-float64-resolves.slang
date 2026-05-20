//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=The core module's `float64_t` typedef resolves and lowers to a 64-bit float spelling on HLSL (`double`) and CUDA (`double`).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `float64_t` (typedef of `double`) as part of the
// core module's base scalar vocabulary. We use it in a buffer
// parameter and FileCheck the per-target 64-bit float spelling.
// Not every backend supports 64-bit floats; HLSL and CUDA do.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<float64_t> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float64_t v = float64_t(tid.x) + 1.0L;
    outBuf[tid.x] = v;
}

// HLSL: double
// CUDA: double
