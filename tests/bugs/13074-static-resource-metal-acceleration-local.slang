// Metal acceleration structures are copyable values. This local remains mutable after control-flow
// lowering, so the test guards the late target check against treating every opaque Metal value like
// a texture or sampler that cannot use the `thread` address space.

//TEST:SIMPLE(filecheck=CHECK): -O0 -target metal -entry main -stage compute -validate-ir

//CHECK: acceleration_structure<{{.*}}> [[SELECTED:selectedScene_[0-9]+]];
//CHECK: if({{.*}}selectSecondScene_
//CHECK: [[SELECTED]] = {{.*}}secondScene_
//CHECK: ->reset({{.*}}([[SELECTED]])

RaytracingAccelerationStructure firstScene;
RaytracingAccelerationStructure secondScene;
RWStructuredBuffer<uint> output;
uniform bool selectSecondScene;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RaytracingAccelerationStructure selectedScene = firstScene;
    if (selectSecondScene)
        selectedScene = secondScene;

    RayDesc ray;
    ray.Origin = 0.0;
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.01;
    ray.TMax = 100.0;

    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> query;
    query.TraceRayInline(selectedScene, RAY_FLAG_NONE, 0xff, ray);
    output[0] = query.Proceed() ? 1 : 0;
}
