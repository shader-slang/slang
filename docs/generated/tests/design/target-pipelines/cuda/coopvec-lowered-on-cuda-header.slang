//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=21c9155c4ad2f346c0598d3001ce01acb069faa9bdcbd07e447c0eb95df61d51
//META: purpose=CUDAHeader is not a named case in the lowerCooperativeVectors switch and falls through the default arm, so -target cuh always lowers CoopVec even when the target caps imply optix_coopvec and the OptixCoopVec primitive does not survive in the header emit.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CUDAHeader-specific arm of the lowerCooperativeVectors
// gate: unlike CUDASource (where `-capability optix_coopvec` suppresses
// the pass and `OptixCoopVec` survives — see the sibling test
// collect-cooperative-metadata-coopvec.slang), CUDAHeader falls through
// the `default` arm and runs the pass unconditionally. The same
// optix_coopvec shader compiled with `-target cuh` must therefore have
// its CoopVec lowered; CHECK-NOT asserts the OptixCoopVec primitive is
// absent from the emitted header text.

//TEST:SIMPLE(filecheck=CHECK):-target cuh -capability optix_coopvec -stage closesthit -entry closestHit

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

// CHECK-DAG: slang-cuda-prelude.h
// CHECK-NOT: OptixCoopVec
