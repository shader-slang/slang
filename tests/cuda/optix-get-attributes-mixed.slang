//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitShader -stage closesthit
//TEST:SIMPLE(filecheck=CHECK_PTX): -target ptx -entry closestHitShader -stage closesthit

//CHECK: optixGetAttribute_0()
//CHECK: __int_as_float(optixGetAttribute_1())
//CHECK: optixGetAttribute_2()
//CHECK: __int_as_float(optixGetAttribute_3())
//CHECK: optixGetAttribute_4()
//CHECK: __int_as_float(optixGetAttribute_5())
//CHECK: optixGetAttribute_6()
//CHECK: __int_as_float(optixGetAttribute_7())
//CHECK_PTX: _optix_get_attribute_0
//CHECK_PTX: _optix_get_attribute_1
//CHECK_PTX: _optix_get_attribute_2
//CHECK_PTX: _optix_get_attribute_3
//CHECK_PTX: _optix_get_attribute_4
//CHECK_PTX: _optix_get_attribute_5
//CHECK_PTX: _optix_get_attribute_6
//CHECK_PTX: _optix_get_attribute_7

struct CustomIntersectionAttributes
{
    uint attr0;   // Will use index 0 - integer
    float attr1;  // Will use index 1 - float
    uint attr2;   // Will use index 2 - integer
    float attr3;  // Will use index 3 - float
    int attr4;    // Will use index 4 - integer
    float attr5;  // Will use index 5 - float
    uint attr6;   // Will use index 6 - integer
    float attr7;  // Will use index 7 - float
};

struct RayPayload
{
    uint uintValues[4];
    float floatValues[4];
};

[shader("closesthit")]
void closestHitShader(inout RayPayload payload, in CustomIntersectionAttributes attr)
{
    payload.uintValues[0] = attr.attr0;   // optixGetAttribute_0()
    payload.floatValues[0] = attr.attr1;  // __int_as_float(optixGetAttribute_1())
    payload.uintValues[1] = attr.attr2;   // optixGetAttribute_2()
    payload.floatValues[1] = attr.attr3;  // __int_as_float(optixGetAttribute_3())
    payload.uintValues[2] = uint(attr.attr4);  // optixGetAttribute_4()
    payload.floatValues[2] = attr.attr5;  // __int_as_float(optixGetAttribute_5())
    payload.uintValues[3] = attr.attr6;   // optixGetAttribute_6()
    payload.floatValues[3] = attr.attr7;  // __int_as_float(optixGetAttribute_7())
}
