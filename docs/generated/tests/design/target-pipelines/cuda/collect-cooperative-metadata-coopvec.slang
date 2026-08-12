//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=4b70b0a669cc9b83124a252d2add0ed8fb3787692f5a88feaade1d3780fda053
//META: purpose=collectCooperativeMetadata fires for CUDA when the target caps imply cooperative_vector; an optix_coopvec CoopVec emit preserves the OptixCoopVec primitive in the CUDA C++ text.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase C row 29: `collectCooperativeMetadata` fires when
// `targetCaps` implies `cooperative_matrix` or `cooperative_vector`.
// Enabling the `optix_coopvec` capability route satisfies that
// predicate and additionally suppresses `lowerCooperativeVectors`
// (see Phase B row 47), so the user-visible consequence is that
// the CoopVec type survives in CUDA emit as `OptixCoopVec<T, N>`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -capability optix_coopvec -stage closesthit -entry closestHit

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=inputVec
ByteAddressBuffer inputVec;

[raypayload]
struct RayPayload
{
    float4 result : read(caller) : write(caller, closesthit);
};

[shader("closesthit")]
void closestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    CoopVec<float, 4> v = coopVecLoad<4, float>(inputVec);
    CoopVec<float, 4> doubled = v + v;
    payload.result.x = doubled[0];
    payload.result.y = doubled[1];
    payload.result.z = doubled[2];
    payload.result.w = doubled[3];
}

// CHECK: OptixCoopVec
