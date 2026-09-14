//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=a3be727c2a77fc867b7d7651d55a75189613e996499f512ef63d1aad430c40fe
//META: purpose=A float4 immutable-buffer load maps directly to a single __ldg because float4 has a corresponding CUDA vector type.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the type-directed half of `lowerImmutableBufferLoadForCUDA`
// for the vector case that maps straight through: `float4` is one of the
// CUDA vector types `__ldg` accepts, so the whole element is fetched by
// one intrinsic call. The CHECK asserts a single `float4` temporary
// assigned from `__ldg(`, and CHECK-NOT rules out the fallback shape
// (a generated `slang_ldg` helper) that the non-mapping case produces.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

StructuredBuffer<float4> src;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = src[tid.x];
}

// CHECK-NOT: slang_ldg
// CHECK: float4 {{[A-Za-z0-9_ ]*}}= __ldg(
