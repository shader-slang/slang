//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
//META: purpose=lowerCooperativeVectors fires on CUDA when the target caps do not imply optix_coopvec, so a CoopVec<T,N> is replaced by a fixed-size array form in emit and the OptixCoopVec primitive does not survive.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase B row 47: `lowerCooperativeVectors` runs for CUDA only
// when `targetCaps` does NOT imply `optix_coopvec` (line ~1457-
// 1462). Without `-capability optix_coopvec`, the pass lowers
// the CoopVec type to an array, so the `OptixCoopVec` primitive
// is absent from CUDA emit and the operands appear as an array
// element walk instead.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=inputVec
ByteAddressBuffer inputVec;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    CoopVec<float, 4> v = coopVecLoad<4, float>(inputVec);
    outBuf[tid.x] = v[0] + v[1] + v[2] + v[3];
}

// CHECK: extern "C" __global__ void main
// CHECK-NOT: OptixCoopVec
