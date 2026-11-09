//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.pipeline_rt;

[raypayload]
struct Payload
{
    uint value : read(caller, closesthit, anyhit)
        : write(caller, closesthit, anyhit);
}
struct Attributes
{
    float2 uv;
}
struct Empty
{
}
[shader("closesthit")]
void closest(inout Payload payload, in Attributes attributes, uniform uint bias)
{
    payload.value += bias;
}
[shader("anyhit")]
void anyHit(inout Payload payload, in Attributes attributes, uniform uint cutoff)
{
    if (payload.value < cutoff)
        IgnoreHit();
}
[shader("intersection")]
void intersection(uniform Empty empty, uniform float distance)
{
    ReportHit(distance, 0, Attributes(float2(0)));
}

[host]
void main()
{
    let pipeline = makeRaytracingPipeline(1);
    __setRayHitGroup(pipeline, 0, 0, Payload, Attributes, closest, anyHit, intersection)(
        1,
        2,
        Empty(),
        3.0);
    __setRayHitGroup(pipeline, 1, 0, Payload, Attributes, void, void, intersection)(Empty(), 4.0);
    __setRayHitGroup(pipeline, 2, 0, Payload, Attributes, closest, void, intersection)(
        5,
        Empty(),
        6.0);
    __setRayHitGroup(pipeline, 3, 0, Payload, Attributes, void, anyHit, intersection)(
        7,
        Empty(),
        8.0);
}
// CHECK-DAG: OpRayHitGroup Procedural {{.*}} ClosestHit {{.*}} AnyHit {{.*}} Intersection
// CHECK-DAG: OpRayHitGroup Procedural {{.*}} Intersection
// CHECK-DAG: OpSetRayHitGroup {{.*}} ClosestHit 2 {{.*}} AnyHit 2 {{.*}} Intersection 1
// CHECK-DAG: OpSetRayHitGroup {{.*}} Intersection 1
