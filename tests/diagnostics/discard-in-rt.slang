//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: 'closestHit' uses capabilities that are not available in 'closesthit' stage.
// CHECK: see using of 'discard'

struct PrimaryRayPayload{}
[require(spvRayTracingKHR)]
[shader("closesthit")]
void closestHit(
    inout PrimaryRayPayload rayData : SV_RayPayload,
    BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes)
{
    discard;
}
