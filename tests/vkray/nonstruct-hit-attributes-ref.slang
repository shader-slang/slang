// A hit-attribute parameter is an entry-point parameter, so it cannot use a reference passing
// mode such as `__ref`, whether its type is a struct or not. The front end rejects it before the
// D3D hit-attribute wrapping pass would see it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile lib_6_6 -whole-program

struct Payload
{
    float4 color;
}

[shader("closesthit")]
void refStruct(inout Payload payload, __ref BuiltInTriangleIntersectionAttributes attributes)
//CHECK:                                                                          ^^^^^^^^^^ E38054
//CHECK:                                                                          ^^^^^^^^^^ entry-point parameter 'attributes' cannot use the reference passing mode 'ref', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    payload.color = float4(attributes.barycentrics, 0.0, 1.0);
}

[shader("closesthit")]
void refVector(inout Payload payload, __ref float2 attributes)
//CHECK:                                           ^^^^^^^^^^ E38054
//CHECK:                                           ^^^^^^^^^^ entry-point parameter 'attributes' cannot use the reference passing mode 'ref', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    payload.color = float4(attributes, 0.0, 1.0);
}
