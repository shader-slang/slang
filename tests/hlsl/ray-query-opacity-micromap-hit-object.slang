//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -profile sm_6_9 -capability ser_dxr

// Both FromRayQuery overloads must infer feature types from an opted-in query.
// Existing explicit generic calls retain their ray-flags/attribute-type ordering.
// CHECK: RayQuery<0{{[uU]? *, *}}1{{[uU]? *}}>
// CHECK: dx::HitObject::FromRayQuery(
// CHECK: .IsHit()
// CHECK: RayQuery<0{{[uU]? *, *}}1{{[uU]? *}}>
// CHECK: dx::HitObject::FromRayQuery({{[^,]+}}, (0{{[uU]?}}),
// CHECK: .IsHit()
// CHECK: RayQuery<0{{[uU]? *}}>
// CHECK: dx::HitObject::FromRayQuery({{[^,]+}}, (0{{[uU]?}}),
// CHECK: .IsHit()

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

struct Attributes
{
    float distance;
}

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 100;
    RayQuery<RAY_FLAG_NONE, RayQueryFeatures<RayQueryOpacityMicroMaps>> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    while (query.Proceed())
    {
        if (query.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)
            query.CommitNonOpaqueTriangleHit();
    }
    HitObject hit = HitObject::FromRayQuery(query);
    output[0] = hit.IsHit();

    Attributes attributes;
    attributes.distance = 1;

    RayQuery<RAY_FLAG_NONE, RayQueryFeatures<RayQueryOpacityMicroMaps>> customQuery;
    customQuery.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    while (customQuery.Proceed())
    {
        if (customQuery.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE)
            customQuery.CommitProceduralPrimitiveHit(attributes.distance);
    }
    HitObject customHit = HitObject::FromRayQuery(customQuery, 0, attributes);
    output[1] = customHit.IsHit();

    RayQuery<RAY_FLAG_NONE> legacyQuery;
    legacyQuery.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    while (legacyQuery.Proceed())
    {
        if (legacyQuery.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE)
            legacyQuery.CommitProceduralPrimitiveHit(attributes.distance);
    }
    HitObject legacyHit = HitObject::FromRayQuery<RAY_FLAG_NONE, Attributes>(
        legacyQuery, 0, attributes);
    output[2] = legacyHit.IsHit();
}
