//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: error 36117:{{.*}}'closestHit'{{.*}}'closesthit'
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

// CHECK: error 36107:{{.*}}'closestHit1'{{.*}}'closesthit'{{.* }}'spirv' 
// CHECK: see using of 'discard'
[shader("closesthit")]
void closestHit1(
    inout PrimaryRayPayload rayData : SV_RayPayload,
    BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes)
{
    discard;
}
