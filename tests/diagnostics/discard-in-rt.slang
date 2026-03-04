//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct PrimaryRayPayload{}
[require(spvRayTracingKHR)]
[shader("closesthit")]
void closestHit(
/*CHECK:
     ^^^^^^^^^^ dependencies not compatible on stage
     ^^^^^^^^^^ 'closestHit' requires support for stage 'closesthit', but stage is unsupported.
     ^^^^^^^^^^ see definition of 'closestHit'
*/
    inout PrimaryRayPayload rayData : SV_RayPayload,
    BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes)
{
    discard;
/*CHECK:
    ^^^^^^^ see using of 'discard'
*/
}

[shader("closesthit")]
void closestHit1(
/*CHECK:
     ^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^ entrypoint 'closestHit1' uses features that are not available in '_closesthit' stage for 'spirv' compilation target.
*/
    inout PrimaryRayPayload rayData : SV_RayPayload,
    BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes)
{
    discard;
/*CHECK:
    ^^^^^^^ see using of 'discard'
*/
}
