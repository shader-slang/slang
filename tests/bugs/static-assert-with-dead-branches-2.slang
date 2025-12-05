//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -minimum-slang-optimization

// CHECK: OpEntryPoint
// CHECK-NOT: error

// Resolution of `defaultGetDescriptorFromHandle` requires clean up of dead-branches,
// otherwise `static_assert` will incorrectly throw.

[vk::push_constant]
cbuffer _PC
{
    RaytracingAccelerationStructure.Handle TEST_BVH;
};

[numthreads(1,1,1)]
void computeMain()
{
    typedef float3 vec3;
    typedef float2 vec2;
    vec2 uvn = vec2(0); 

    float t = 0.;

    RayDesc ray;
    ray.Origin = vec3(0,0,0);
    ray.TMin = 0.01f;
    ray.Direction = vec3(0,0,1);
    ray.TMax = 1e4f;

    RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
                    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
                    
    uint rayFlags = RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
                    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;

    q.TraceRayInline( TEST_BVH, rayFlags, 0xff, ray );
}