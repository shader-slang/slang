// Structural ray-tracing API proof of concept.

import rt_pipeline;

struct RadiancePayload
{
    float4 color;
}

struct SphereIntersectionAttributes
{
    float2 uv;
}

struct PrimaryHitRecord {}
struct PrimaryMissRecord {}

struct PrimaryTraceContext : rt::ITraceContext
{
    typealias Payload = RadiancePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct PrimaryTriangleContext : rt::IHitContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = PrimaryHitRecord;
}

struct PrimaryCurveContext : rt::IHitContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::CurvePrimitive;
    typealias Record = PrimaryHitRecord;
}

struct PrimarySphereContext : rt::IHitContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<SphereIntersectionAttributes>;
    typealias Record = PrimaryHitRecord;
}

struct PrimaryMissContext : rt::IMissGroupContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Record = PrimaryMissRecord;
}

// Each stage is a named struct entry-point candidate. Its invoke() method is
// retained and adapted only when the stage is selected explicitly or through a
// concrete program layout.
struct PrimaryMiss : rt::IMissShader<PrimaryMissContext>
{
    void invoke(rt::MissInput<PrimaryMissContext> input)
    {
        input.payload.color = float4(0.0, 0.0, 0.0, 1.0);
    }
}

struct PrimaryTriangleClosestHit : rt::IClosestHitShader<PrimaryTriangleContext>
{
    void invoke(rt::ClosestHitInput<PrimaryTriangleContext> input)
    {
        input.payload.color =
            float4(input.triangle.barycentricCoord, input.distance, 1.0);
    }
}

struct PrimaryTriangleAnyHit : rt::IAnyHitShader<PrimaryTriangleContext>
{
    void invoke(rt::AnyHitInput<PrimaryTriangleContext> input)
    {
        if (!input.triangle.frontFacing)
        {
            input.ignoreHit();
        }
    }
}

struct PrimaryCurveClosestHit : rt::IClosestHitShader<PrimaryCurveContext>
{
    void invoke(rt::ClosestHitInput<PrimaryCurveContext> input)
    {
        input.payload.color = float4(input.curve.parameter, input.distance, 0.0, 1.0);
    }
}

struct PrimarySphereIntersection
    : rt::IIntersectionShader<PrimarySphereContext>
{
    void invoke(rt::IntersectionInput<PrimarySphereContext> input)
    {
        SphereIntersectionAttributes attributes;
        attributes.uv = float2(0.25, 0.75);
        input.reportHit(1.0, attributes);
    }
}

struct PrimarySphereClosestHit : rt::IClosestHitShader<PrimarySphereContext>
{
    void invoke(rt::ClosestHitInput<PrimarySphereContext> input)
    {
        SphereIntersectionAttributes attributes = input.attributes;
        input.payload.color = float4(attributes.uv, input.distance, 1.0);
    }
}

// Slots are indices within their respective logical SBT sections.
struct PrimaryMissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = PrimaryMissContext;
    typealias Miss = PrimaryMiss;
}

struct PrimaryTriangleGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = PrimaryTriangleContext;
    typealias ClosestHit = PrimaryTriangleClosestHit;
    typealias AnyHit = PrimaryTriangleAnyHit;
    typealias Intersection = rt::NoIntersection<PrimaryTriangleContext>;
}

struct PrimaryCurveGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<1>;
    typealias Context = PrimaryCurveContext;
    typealias ClosestHit = PrimaryCurveClosestHit;
    typealias AnyHit = rt::NoAnyHit<PrimaryCurveContext>;
    typealias Intersection = rt::NoIntersection<PrimaryCurveContext>;
}

struct PrimarySphereGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<2>;
    typealias Context = PrimarySphereContext;
    typealias ClosestHit = PrimarySphereClosestHit;
    typealias AnyHit = rt::NoAnyHit<PrimarySphereContext>;
    typealias Intersection = PrimarySphereIntersection;
}

struct PrimaryTraceProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = PrimaryTraceContext;
    typealias MissGroups = rt::MissGroupList<TraceContext, PrimaryMissGroup>;
    typealias HitGroups = rt::HitGroupList<
        TraceContext,
        PrimaryTriangleGroup,
        PrimaryCurveGroup,
        PrimarySphereGroup>;
    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

rt::AccelerationStructure gScene;
rt::TraceProgramDescriptor<PrimaryTraceProgramLayout> gPrimaryDescriptor;

rt::RayDesc makePrimaryRay(uint2 pixel)
{
    rt::RayDesc ray;
    ray.origin = float3(0.0, 0.0, -1.0);
    ray.tMin = 0.001;
    ray.direction = normalize(float3(float(pixel.x), float(pixel.y), 1.0));
    ray.tMax = 10000.0;
    return ray;
}

[shader("raygeneration")]
void rayGenMain()
{
    uint2 pixel = DispatchRaysIndex().xy;

    rt::RayTraversalDesc desc;
    desc.ray = makePrimaryRay(pixel);
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 3;
    desc.missIndex = 0;

    RadiancePayload payload;
    payload.color = float4(0.0, 0.0, 0.0, 1.0);

    rt::RayTracer<PrimaryTraceProgramLayout> tracer;
    tracer.trace(desc, gScene, gPrimaryDescriptor, payload);
}

// Conceptual Metal lowering:
//
// result = intersector.intersect(desc.ray, gScene, descriptor.ift, ...);
// if (result.type == none)
// {
//     descriptor.missFunctions[desc.missIndex](payload, descriptor.recordData);
// }
// else
// {
//     uint hitSlot = getHitGroupSlot(
//         desc,
//         getGeometryContribution(result),
//         getInstanceContribution(result));
//     descriptor.closestHitFunctions[hitSlot](
//         payload,
//         result,
//         descriptor.recordData);
// }
//
// The descriptor specializes to the Metal IFT, visible-function tables, and
// generated record-data buffer. D3D/Vulkan instead reuse desc's native SBT
// parameters and emit the selected structs as ordinary ray-tracing stages.
