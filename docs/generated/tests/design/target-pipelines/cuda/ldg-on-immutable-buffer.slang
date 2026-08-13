//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=a3be727c2a77fc867b7d7651d55a75189613e996499f512ef63d1aad430c40fe
//META: purpose=lowerImmutableBufferLoadForCUDA translates a StructuredBuffer<T> load into a __ldg intrinsic call so the read-only cache path is used.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `lowerImmutableBufferLoadForCUDA` fires for `isCUDATarget` in
// Phase C. Loads of a read-only (StructuredBuffer) element become
// `__ldg(...)` calls in the emitted CUDA C++ text.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

StructuredBuffer<float> roBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(roBuf[tid.x]);
}

// CHECK: __ldg
