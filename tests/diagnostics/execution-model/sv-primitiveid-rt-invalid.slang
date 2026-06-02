// Regression tests for the SV_PrimitiveID ray-tracing allowlist. Only
// input SV_PrimitiveID parameters in intersection, any-hit, and closest-hit
// stages are accepted.

struct Payload
{
    float value;
};

//DIAGNOSTIC_TEST:SIMPLE(diag=RAYGEN):-target hlsl -entry raygenMain -stage raygeneration
//DIAGNOSTIC_TEST:SIMPLE(diag=MISS):-target hlsl -entry missMain -stage miss
//DIAGNOSTIC_TEST:SIMPLE(diag=CALLABLE):-target hlsl -entry callableMain -stage callable
//DIAGNOSTIC_TEST:SIMPLE(diag=ISECT_OUT):-target hlsl -entry intersectionOut -stage intersection
//DIAGNOSTIC_TEST:SIMPLE(diag=INOUT):-target hlsl -entry anyhitInOut -stage anyhit
//DIAGNOSTIC_TEST:SIMPLE(diag=CHIT_INOUT):-target hlsl -entry closesthitInOut -stage closesthit

[shader("raygeneration")]
void raygenMain(uint primitiveID : SV_PrimitiveID)
//RAYGEN:            ^^^^^^^^^^^ system value semantic 'SV_PrimitiveID' cannot be used as input in 'raygeneration' shader stage
{
}

[shader("miss")]
void missMain(uint primitiveID : SV_PrimitiveID)
//MISS:            ^^^^^^^^^^^ system value semantic 'SV_PrimitiveID' cannot be used as input in 'miss' shader stage
{
}

[shader("callable")]
void callableMain(uint primitiveID : SV_PrimitiveID)
//CALLABLE:            ^^^^^^^^^^^ system value semantic 'SV_PrimitiveID' cannot be used as input in 'callable' shader stage
{
}

[shader("anyhit")]
void anyhitInOut(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes,
    inout uint primitiveID : SV_PrimitiveID)
//INOUT:       ^^^^^^^^^^^ system value semantic 'SV_PrimitiveID' cannot be used as output in 'anyhit' shader stage
{
    primitiveID = uint(attribs.barycentrics.x + payload.value);
}

[shader("intersection")]
void intersectionOut(out uint primitiveID : SV_PrimitiveID)
//ISECT_OUT:                  ^^^^^^^^^^^ system value semantic 'SV_PrimitiveID' cannot be used as output in 'intersection' shader stage
{
    primitiveID = 0;
}

[shader("closesthit")]
void closesthitInOut(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    inout uint primitiveID : SV_PrimitiveID)
//CHIT_INOUT:  ^^^^^^^^^^^ system value semantic 'SV_PrimitiveID' cannot be used as output in 'closesthit' shader stage
{
    primitiveID = uint(attribs.barycentrics.x + payload.value);
}
