//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=a3be727c2a77fc867b7d7651d55a75189613e996499f512ef63d1aad430c40fe
//META: purpose=A type too large for a direct __ldg gets a generated per-type load function name-hinted slang_ldg, which the CUDA emitter renders as an ordinary function call.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third arm of the type-directed rewrite in
// `lowerImmutableBufferLoadForCUDA`. The buffer element is a five-field
// struct of float4s — larger than anything `__ldg` takes directly — so
// the pass synthesizes a per-type loader instead of inlining the reads
// at the use site. The CHECKs assert the generated `__device__`
// `slang_ldg` definition, the per-field `__ldg` reads inside it, and the
// ordinary call to it from the kernel body.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

struct Big
{
    float4 a;
    float4 b;
    float4 c;
    float4 d;
    float4 e;
};

StructuredBuffer<Big> src;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Big v = src[tid.x];
    outBuf[tid.x] = v.a.x + v.e.w;
}

// The generated per-type loader ...
// CHECK: __device__ {{.*}} slang_ldg
// ... reads each field through __ldg ...
// CHECK: __ldg(&{{[A-Za-z0-9_]+}}->a_
// CHECK: __ldg(&{{[A-Za-z0-9_]+}}->e_
// ... and the kernel body calls it like any other function.
// CHECK: = slang_ldg
