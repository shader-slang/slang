//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hgbc

[raypayload]
struct Payload
{
    float4 color : read(caller) : write(caller, closesthit, miss);
};

[shader("raygeneration")]
void rayGeneration(uniform RaytracingAccelerationStructure scene)
{}

[shader("closesthit")]
void closestHit(inout Payload payload)
{}

[shader("miss")]
void miss(inout Payload payload)
{}

[host]
void main()
{
    let tracer = raytrace(rayGeneration, closestHit, miss);
//CHECK:                                 ^^^^^^^^^^ invalid ray-tracing shader signature
//CHECK:                                 ^^^^^^^^^^ closest-hit shader must accept triangle intersection attributes
}
