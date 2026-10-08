// D3D receives hit attributes only through an `in` parameter. A `__ref` attribute is not turned
// into `borrow in`, so the pass leaves it unchanged, struct or not, and DXC reports that the
// parameter must be `in`, as before wrappers were added; the compiler must not fail internally.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile lib_6_6 -whole-program -validate-ir-detailed

struct Payload
{
    float4 color;
}

[shader("closesthit")]
void refStruct(inout Payload payload, __ref BuiltInTriangleIntersectionAttributes attributes)
{
    payload.color = float4(attributes.barycentrics, 0.0, 1.0);
}

[shader("closesthit")]
void refVector(inout Payload payload, __ref float2 attributes)
{
    payload.color = float4(attributes, 0.0, 1.0);
}

// HLSL-NOT: HitAttributes_t
// HLSL: void refStruct(inout Payload{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, inout BuiltInTriangleIntersectionAttributes {{[A-Za-z0-9_]+}})
// HLSL: void refVector(inout Payload{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, inout float2 {{[A-Za-z0-9_]+}})
