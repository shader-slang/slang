//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc

import slang.pipeline_rt;

[raypayload]
struct Payload
{
    uint value : read(caller, closesthit, miss)
        : write(caller, closesthit, miss);
}

[shader("raygeneration")]
void generate() {}

[shader("closesthit")]
void hitOut(inout Payload payload, out BuiltInTriangleIntersectionAttributes attributes)
{
    attributes.barycentrics = float2(0);
}

[shader("closesthit")]
void hitInOut(inout Payload payload, inout BuiltInTriangleIntersectionAttributes attributes) {}

[host]
void main()
{
    let pipeline = makeRaytracingPipeline(1);
    __setRayMiss(pipeline, 0, generate)();
    __setRayHitShader(pipeline, 0, 0, hitOut)();
    __setRayHitShader(pipeline, 0, 0, hitInOut)();
}

// CHECK: miss shader must name an entry point with the matching shader stage
// CHECK: closest-hit shader hit attributes must be passed by value as an input parameter
// CHECK: closest-hit shader hit attributes must be passed by value as an input parameter
