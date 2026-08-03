// Part 3 alternative example: structural dispatch table usage.
//
// This sketch asks the shader author to provide the selection structure that is
// otherwise implicit in a native SBT. Metal can lower that structure to ordinary
// shader-side post-intersect code, while D3D/Vulkan can lower or reflect it as
// ordinary miss/hit-group entry points.

import rt_pipeline;

struct RadiancePayload
{
    float4 color;
}

struct SphereHitAttributes
{
    float2 uv;
}

struct PrimaryTraceContext : rt::ITraceContext
{
    typealias Payload = RadiancePayload;
    typealias ASKind = rt::InstanceAS;
    typealias Motion = rt::NoMotion;

    static const rt::RayDataTags dataTags =
        rt::RayDataTags::Triangle | rt::RayDataTags::Curve;
    static const int maxLevels = 0;
}

struct PrimaryTriangleContext : rt::IHitContextFor<PrimaryTraceContext>
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
}

struct PrimaryCurveContext : rt::IHitContextFor<PrimaryTraceContext>
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::CurvePrimitive;
}

struct PrimarySphereContext : rt::IHitContextFor<PrimaryTraceContext>
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive;
}

struct PrimaryMiss : rt::IMissShader<PrimaryTraceContext>
{
    void operator()(rt::MissInput<PrimaryTraceContext> input)
    {
        input.payload.color = float4(0.0, 0.0, 0.0, 1.0);
    }
}

struct PrimaryTriangleClosestHit : rt::IClosestHitShader<PrimaryTriangleContext>
{
    void operator()(rt::ClosestHitInput<PrimaryTriangleContext> input)
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
    void operator()(rt::ClosestHitInput<PrimaryCurveContext> input)
    {
        input.payload.color = float4(input.curve.parameter, input.distance, 0.0, 1.0);
    }
}

struct PrimarySphereIntersection : rt::IIntersectionShader<PrimarySphereContext, SphereHitAttributes>
{
    typealias Attributes = SphereHitAttributes;

    rt::IntersectionReturn<PrimarySphereContext, Attributes> operator()(
        rt::IntersectionInput<PrimarySphereContext> input)
    {
        rt::IntersectionReturn<PrimarySphereContext, Attributes> result;
        result.acceptIntersection = input.opaque;
        result.continueSearch = true;
        result.distance = 1.0;
        result.attributes.uv = float2(0.25, 0.75);
        return result;
    }
}

struct PrimarySphereClosestHit : rt::IClosestHitShader<PrimarySphereContext>
{
    void operator()(rt::ClosestHitInput<PrimarySphereContext> input)
    {
        SphereHitAttributes attr = input.getCustomAttributes<SphereHitAttributes>();
        input.payload.color = float4(attr.uv, input.distance, 1.0);
    }
}

// These are compiled shader-slot IDs, not scene geometry IDs. The compiled
// program has a finite set of hit groups. D3D/Vulkan select these slots through
// native SBT indexing, and Metal lowering synthesizes the same indexing formula
// after intersector::intersect(...) returns.
typealias PrimaryTriangleHitSlot = rt::HitGroupSlot<0>;
typealias PrimaryCurveHitSlot = rt::HitGroupSlot<1>;
typealias PrimarySphereHitSlot = rt::HitGroupSlot<2>;

struct PrimaryProgram : rt::TraceProgram
{
    typealias TraceContext = PrimaryTraceContext;

    typealias MissGroups = rt::MissGroupList<
        TraceContext,

        // dispatch.missIndex == 0
        rt::MissGroup<
            TraceContext,
            rt::MissSlot<0>,
            PrimaryMiss>>;

    typealias HitGroups = rt::HitGroupList<
        TraceContext,

        // hit slot 0. Runtime geometry/ray-type data may select this slot.
        rt::HitGroup<
            TraceContext,
            PrimaryTriangleHitSlot,
            PrimaryTriangleContext,
            PrimaryTriangleClosestHit,
            PrimaryTriangleAnyHit,
            rt::NoAttributes,
            rt::NoIntersection<PrimaryTriangleContext>>,

        // hit slot 1. Runtime geometry/ray-type data may select this slot.
        rt::HitGroup<
            TraceContext,
            PrimaryCurveHitSlot,
            PrimaryCurveContext,
            PrimaryCurveClosestHit,
            rt::NoAnyHit<PrimaryCurveContext>,
            rt::NoAttributes,
            rt::NoIntersection<PrimaryCurveContext>>,

        // hit slot 2. Runtime geometry/ray-type data may select this slot.
        rt::HitGroup<
            TraceContext,
            PrimarySphereHitSlot,
            PrimarySphereContext,
            PrimarySphereClosestHit,
            rt::NoAnyHit<PrimarySphereContext>,
            SphereHitAttributes,
            PrimarySphereIntersection>>;
}

rt::AccelerationStructure<rt::InstanceAS, rt::NoMotion> gScene;

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

    rt::RayDispatch dispatch;
    dispatch.sbtOffset = 0;
    dispatch.sbtStride = 3;
    dispatch.missIndex = 0;

    RadiancePayload payload;
    payload.color = float4(0.0, 0.0, 0.0, 1.0);

    rt::RayTracer<PrimaryProgram> tracer;
    tracer.trace(desc, gScene, dispatch, payload);
}



// Conceptual Metal lowering:
//
// metalIntersector.set_geometry_multiplier(dispatch.sbtStride);
// metalIntersector.set_base_id(dispatch.sbtOffset);
// result = metalIntersector.intersect(desc.ray, gScene, ...);
//
// if (result.type == none)
// {
//     switch (dispatch.missIndex)
//     {
//     case 0: PrimaryMiss()(MissInput<PrimaryTraceContext>(payload)); break;
//     }
// }
// else
// {
//     uint geometryContribution =
//         __rtGetGeometryContribution<PrimaryProgram>(result);
//     uint instanceContribution =
//         __rtGetInstanceContribution<PrimaryProgram>(result);
//     uint slot = getHitGroupSlot(
//         dispatch, geometryContribution, instanceContribution);
//
//     switch (slot)
//     {
//     case 0:
//         PrimaryTriangleClosestHit()(
//             ClosestHitInput<PrimaryTriangleContext>(payload, result));
//         break;
//     case 1:
//         PrimaryCurveClosestHit()(
//             ClosestHitInput<PrimaryCurveContext>(payload, result));
//         break;
//     case 2:
//         PrimarySphereClosestHit()(
//             ClosestHitInput<PrimarySphereContext>(payload, result));
//         break;
//     }
// }
//
// Conceptual D3D/Vulkan lowering:
//
// - dispatch.sbtOffset, dispatch.sbtStride, and dispatch.missIndex lower to
//   native trace parameters.
// - MissGroups[0] becomes the native miss shader for miss slot 0.
// - HitGroups[0..2] become native SBT hit groups with closest-hit, optional
//   any-hit, and optional intersection entry points.
// - Existing host-side SBT construction can keep using the same slot numbers.
//
// Metal consistency requirement:
//
// - For intersection_function_buffer, dispatch.sbtStride must be the value used
//   with set_geometry_multiplier(...), and dispatch.sbtOffset must be the value
//   used with set_base_id(...). The synthesized closest-hit slot and Metal's IFB
//   record index are then the same.
// - For ordinary intersection_function_table, Metal has no shader-side base ID
//   or geometry multiplier. It can use this model only when the acceleration
//   structure's function-table offsets are already baked to the same slot
//   numbers, or when the trace uses the restricted stride/offset pattern that
//   those baked offsets represent.
