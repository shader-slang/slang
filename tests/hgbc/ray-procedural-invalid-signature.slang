//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc

import slang.pipeline_rt;

[raypayload]
struct Payload
{
    uint value : read(caller, closesthit)
        : write(caller, closesthit);
}
[raypayload]
struct OtherPayload
{
    uint value : read(caller, closesthit)
        : write(caller, closesthit);
}
struct Attributes
{
    float2 uv;
}
struct OtherAttributes
{
    float2 uv;
}
[shader("closesthit")]
void closest(inout Payload payload, in Attributes attributes) {}
[shader("intersection")]
void intersection(uniform float distance)
{
    ReportHit(distance, 0, Attributes(float2(0)));
}
[shader("intersection")]
void nonuniformIntersection(float distance)
{
    ReportHit(distance, 0, Attributes(float2(0)));
}

[host]
void main()
{
    let pipeline = makeRaytracingPipeline(1);
    __setRayHitGroup(pipeline, 0, 0, OtherPayload, Attributes, closest, void, intersection)(1.0);
    __setRayHitGroup(pipeline, 0, 0, Payload, OtherAttributes, closest, void, intersection)(1.0);
    __setRayHitGroup(pipeline, 0, 0, Payload, Attributes, closest, intersection, void)(1.0);
    __setRayHitGroup(pipeline, 0, 0, Payload, Attributes, void, void, nonuniformIntersection)(1.0);
}
// CHECK: shader payload must match the declared hit-group payload
// CHECK: shader attributes must match the declared hit-group attributes
// CHECK: must name an entry point with the matching shader stage
// CHECK: extra shader parameters must be explicitly uniform
