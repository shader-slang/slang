// Regression test for https://github.com/shader-slang/slang/issues/13265
// On OptiX, GeometryIndex() is defined as the SBT GAS index (see "Geometry index" in
// docs/cuda-target.md). gl_GeometryIndexEXT forwards to GeometryIndex(), so its CUDA run is enough.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry closestHitMain -stage closesthit
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry anyHitMain -stage anyhit
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry intersectionMain -stage intersection
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry glslClosestHitMain -stage closesthit
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry closestHitMain -stage closesthit
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry anyHitMain -stage anyhit
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry intersectionMain -stage intersection

//CHECK: optixGetSbtGASIndex()
//CHECK-PTX: _optix_read_sbt_gas_idx

import glsl;

struct Payload
{
    uint geometryIndex;
};

struct Attributes
{
    uint geometryIndex;
};

[shader("closesthit")]
void closestHitMain(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.geometryIndex = GeometryIndex();
}

[shader("anyhit")]
void anyHitMain(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    if (GeometryIndex() != 0)
        IgnoreHit();
}

[shader("intersection")]
void intersectionMain()
{
    Attributes attributes;
    attributes.geometryIndex = GeometryIndex();
    ReportHit(RayTCurrent(), 0, attributes);
}

[shader("closesthit")]
void glslClosestHitMain(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.geometryIndex = gl_GeometryIndexEXT;
}
