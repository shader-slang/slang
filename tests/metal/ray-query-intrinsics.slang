//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -output-using-type -render-feature ray-query

// MTL-DAG: get_candidate_instance_id
// MTL-DAG: get_candidate_user_instance_id
// MTL-DAG: get_candidate_primitive_id
// MTL-DAG: get_candidate_geometry_id
// MTL-DAG: get_candidate_ray_origin
// MTL-DAG: get_candidate_ray_direction
// MTL-DAG: is_candidate_triangle_front_facing
// MTL-DAG: get_candidate_triangle_barycentric_coord
// MTL-DAG: get_candidate_object_to_world_transform
// MTL-DAG: get_candidate_world_to_object_transform
// MTL-DAG: get_committed_instance_id
// MTL-DAG: get_committed_user_instance_id
// MTL-DAG: get_committed_primitive_id
// MTL-DAG: get_committed_geometry_id
// MTL-DAG: get_committed_ray_origin
// MTL-DAG: get_committed_ray_direction
// MTL-DAG: is_committed_triangle_front_facing
// MTL-DAG: get_committed_triangle_barycentric_coord
// MTL-DAG: get_committed_object_to_world_transform
// MTL-DAG: get_committed_world_to_object_transform

// LIB: computeMain

//TEST_INPUT: set accelStruct = AccelerationStructure
uniform RaytracingAccelerationStructure accelStruct;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = float3(0.1f, 0.1f, 0.0f);
    ray.Direction = float3(0.0f, 0.0f, 1.0f);
    ray.TMin = 0.01f;
    ray.TMax = 1e4f;

    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rq;
    rq.TraceRayInline(accelStruct, RAY_FLAG_NONE, 0xff, ray);

    uint score = 0;
    if (rq.Proceed())
    {
        if (rq.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)
        {
            if (rq.CandidateInstanceIndex() == 0)
                score += 1;

            if (rq.CandidateInstanceID() == 0)
                score += 1;

            if (rq.CandidatePrimitiveIndex() == 0)
                score += 1;

            if (rq.CandidateGeometryIndex() == 0)
                score += 1;

            float3 oro = rq.CandidateObjectRayOrigin();
            if (oro.x == 0.1f && oro.y == 0.1f && oro.z == 0.0f)
                score += 1;

            float3 ord = rq.CandidateObjectRayDirection();
            if (ord.x == 0.0f && ord.y == 0.0f && ord.z == 1.0f)
                score += 1;

            if (rq.CandidateTriangleFrontFace() == true)
                score += 1;

            float2 bary = rq.CandidateTriangleBarycentrics();
            if (bary.x != 0.0f && bary.y != 0.0f)
                score += 1;

            if (rq.CandidateObjectToWorld3x4()._11 == 1.0f)
                score += 1;

            if (rq.CandidateObjectToWorld4x3()._11 == 1.0f)
                score += 1;

            if (rq.CandidateWorldToObject3x4()._11 == 1.0f)
                score += 1;

            if (rq.CandidateWorldToObject4x3()._11 == 1.0f)
                score += 1;

            rq.CommitNonOpaqueTriangleHit();
            rq.Abort();
        }
    }

    if (rq.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        if (rq.CommittedInstanceIndex() == 0)
            score += 1;

        if (rq.CommittedInstanceID() == 0)
            score += 1;

        if (rq.CommittedPrimitiveIndex() == 0)
            score += 1;

        if (rq.CommittedGeometryIndex() == 0)
            score += 1;

        float3 oro = rq.CommittedObjectRayOrigin();
        if (oro.x == 0.1f && oro.y == 0.1f && oro.z == 0.0f)
            score += 1;

        float3 ord = rq.CommittedObjectRayDirection();
        if (ord.x == 0.0f && ord.y == 0.0f && ord.z == 1.0f)
            score += 1;

        if (rq.CommittedTriangleFrontFace() == true)
            score += 1;

        float2 bary = rq.CommittedTriangleBarycentrics();
        if (bary.x != 0.0f && bary.y != 0.0f)
            score += 1;

        if (rq.CommittedObjectToWorld3x4()._11 == 1.0f)
            score += 1;

        if (rq.CommittedObjectToWorld4x3()._11 == 1.0f)
            score += 1;

        if (rq.CommittedWorldToObject3x4()._11 == 1.0f)
            score += 1;

        if (rq.CommittedWorldToObject4x3()._11 == 1.0f)
            score += 1;
    }

    outputBuffer[0] = score;
}

// CHECK: 24
