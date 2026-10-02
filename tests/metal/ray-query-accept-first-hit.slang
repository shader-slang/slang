// Tests that RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH reaches Metal's intersection_params,
// both as a TraceRayInline argument and as a RayQuery template flag, and that RayFlags()
// reconstructs it (shader-slang/slang#13408).

//TEST:SIMPLE(filecheck=FWD): -target metal -stage compute -entry runtimeFlag
//TEST:SIMPLE(filecheck=REV): -target metal -stage compute -entry runtimeFlag
//TEST:SIMPLE(filecheck=TEMPLATE): -target metal -stage compute -entry templateFlag
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry runtimeFlag
//TEST:SIMPLE(filecheck=LIB-TEMPLATE): -target metallib -stage compute -entry templateFlag
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -entry runtimeFlag -output-using-type -render-feature ray-query
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry runtimeFlag -output-using-type -render-feature ray-query

// FWD: _slang_ray_flags_to_intersection_params(uint flags)
// FWD: if (flags & 0x04) /* RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH */
// FWD-NEXT: params.accept_any_intersection(true);
// FWD: _slang_ray_flags_to_intersection_params((4U))

// REV: _slang_intersection_params_to_ray_flags(raytracing::intersection_params params)
// REV: if (params.should_accept_any_intersection())
// REV-NEXT: flags |= 0x04; /* RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH */
// REV: _slang_intersection_params_to_ray_flags((&

// TEMPLATE: if (flags & 0x04) /* RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH */
// TEMPLATE-NEXT: params.accept_any_intersection(true);
// TEMPLATE: _slang_ray_flags_to_intersection_params((4U))

// LIB: runtimeFlag
// LIB-TEMPLATE: templateFlag

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name result
RWStructuredBuffer<uint> result;

// The test acceleration structure holds one opaque triangle at z = 0.5 covering this ray.
RayDesc makeRay()
{
    RayDesc ray;
    ray.Origin = float3(0.1, 0.1, 0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.01;
    ray.TMax = 1e4;
    return ray;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void runtimeFlag()
{
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, makeRay());
    q.Proceed();
    result[0] = q.CommittedStatus();
    result[1] = q.RayFlags();
}

// BUF: 1
// BUF-NEXT: 4

[shader("compute")]
[numthreads(1, 1, 1)]
void templateFlag()
{
    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(scene, RAY_FLAG_NONE, 0xFF, makeRay());
    q.Proceed();
    result[0] = q.CommittedStatus();
}
