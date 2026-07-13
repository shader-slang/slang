//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:40:21+00:00
//META: source_commit=4e394f5c941b8145021611e1d3016b1f27a4ba52
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=0410773935d5331fbece0da3f2383f2a7478f2afa84b7d440d2490b07f25489f
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
