//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc

[raypayload]
struct Payload
{
    float4 color : read(caller) : write(caller, closesthit, miss);
};

[shader("raygeneration")]
void rayGeneration()
{}

[shader("closesthit")]
void closestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{}

[shader("miss")]
void miss(inout Payload payload)
{}

void nonConstant(int recursionDepth)
{
    raytrace(rayGeneration, closestHit, miss, recursionDepth);
}

void zeroDepth()
{
    raytrace(rayGeneration, closestHit, miss, 0);
}

// CHECK: expression does not evaluate to a compile-time constant
// CHECK: 0 is not a valid ray-tracing recursion depth; expected at least 1
