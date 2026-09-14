//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=a3be727c2a77fc867b7d7651d55a75189613e996499f512ef63d1aad430c40fe
//META: purpose=A half3 immutable-buffer load has no corresponding CUDA vector type, so the pass emits one __ldg per element and reassembles the vector.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "otherwise" branch of the vector rule in
// `lowerImmutableBufferLoadForCUDA`: a half-precision 3-vector is not a
// CUDA vector type `__ldg` accepts, so the element is fetched component
// by component and rebuilt. The CHECKs assert three per-component
// `__ldg(&(...->x/y/z))` reads followed by the reassembly call, which is
// the emitted shape only the per-element path produces.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

StructuredBuffer<half3> src;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    half3 v = src[tid.x];
    outBuf[tid.x] = float(v.x) + float(v.y) + float(v.z);
}

// CHECK-DAG: __ldg(&({{[A-Za-z0-9_]+}}->x))
// CHECK-DAG: __ldg(&({{[A-Za-z0-9_]+}}->y))
// CHECK-DAG: __ldg(&({{[A-Za-z0-9_]+}}->z))
// CHECK-DAG: make___half3
