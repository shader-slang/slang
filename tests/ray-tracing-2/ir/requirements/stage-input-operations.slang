//TEST:SIMPLE(filecheck=CH): -experimental-feature -entry ClosestHit -stage closesthit -dump-ir -no-codegen
//TEST:SIMPLE(filecheck=AH): -experimental-feature -entry AnyHit -stage anyhit -dump-ir -no-codegen
//TEST:SIMPLE(filecheck=IS): -experimental-feature -entry Intersection -stage intersection -dump-ir -no-codegen
//TEST:SIMPLE(filecheck=MS): -experimental-feature -entry Miss -stage miss -dump-ir -no-codegen

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TriangleContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct CustomAttributes
{
    float value;
}

struct ProceduralContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
    typealias Record = TestRecord;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = TestRecord;
}

RWStructuredBuffer<float4> output;

struct ClosestHit : rt::IClosestHitShader<TriangleContext>
{
    void invoke(rt::ClosestHitInput<TriangleContext> input)
    {
        output[0] = float4(
            input.worldSpaceOrigin.x + input.worldSpaceDirection.x,
            input.distance,
            float(input.hitKind),
            1.0);
    }
}

// CH: structuralRayTracingGetWorldRayOrigin
// CH: structuralRayTracingGetWorldRayDirection
// CH: structuralRayTracingGetRayTCurrent
// CH: structuralRayTracingGetHitKind

struct AnyHit : rt::IAnyHitShader<TriangleContext>
{
    void invoke(rt::AnyHitInput<TriangleContext> input)
    {
        if (input.worldSpaceOrigin.x + input.worldSpaceDirection.x + input.distance > 0.0)
            input.ignoreHit();
        if (input.hitKind != 0)
            input.acceptHitAndEndSearch();
    }
}

// AH: structuralRayTracingGetWorldRayOrigin
// AH: structuralRayTracingGetWorldRayDirection
// AH: structuralRayTracingGetRayTCurrent
// AH: structuralRayTracingIgnoreHit
// AH: structuralRayTracingGetHitKind
// AH: structuralRayTracingAcceptHitAndEndSearch

struct Intersection : rt::IIntersectionShader<ProceduralContext>
{
    void invoke(rt::IntersectionInput<ProceduralContext> input)
    {
        let ray = input.objectSpaceRay;
        CustomAttributes attributes;
        attributes.value = ray.origin.x + ray.direction.x + input.worldSpaceOrigin.x +
                           input.worldSpaceDirection.x + float(input.primitiveIndex) +
                           float(input.geometryIndex);
        if (!input.reportHit(ray.tMin, attributes))
            input.reportHit(ray.tMax, 1, attributes);
    }
}

// IS: structuralRayTracingGetObjectSpaceRay
// IS: structuralRayTracingGetWorldRayOrigin
// IS: structuralRayTracingGetWorldRayDirection
// IS: structuralRayTracingGetPrimitiveIndex
// IS: structuralRayTracingGetGeometryIndex
// IS: structuralRayTracingReportHit
// IS: structuralRayTracingReportHitWithKind

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        output[0] = float4(input.worldSpaceOrigin + input.worldSpaceDirection, 1.0);
    }
}

// MS: structuralRayTracingGetWorldRayOrigin
// MS: structuralRayTracingGetWorldRayDirection
