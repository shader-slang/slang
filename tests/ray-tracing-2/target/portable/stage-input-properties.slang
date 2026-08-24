//TEST:SIMPLE(filecheck=HLSLCH): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=HLSLAH): -experimental-feature -entry AnyHit -stage anyhit -target hlsl
//TEST:SIMPLE(filecheck=HLSLIS): -experimental-feature -entry Intersection -stage intersection -target hlsl
//TEST:SIMPLE(filecheck=HLSLMS): -experimental-feature -entry Miss -stage miss -target hlsl
//TEST:SIMPLE(filecheck=SPVCH): -experimental-feature -entry ClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVAH): -experimental-feature -entry AnyHit -stage anyhit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVIS): -experimental-feature -entry Intersection -stage intersection -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVMS): -experimental-feature -entry Miss -stage miss -target spirv-assembly -emit-spirv-directly

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

// HLSLCH-DAG: WorldRayOrigin()
// HLSLCH-DAG: WorldRayDirection()
// HLSLCH-DAG: RayTCurrent()
// HLSLCH-DAG: HitKind()
// HLSLCH: [shader("closesthit")]void ClosestHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, StructuralTriangleHitAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// SPVCH: OpEntryPoint ClosestHitKHR
// SPVCH-DAG: BuiltIn WorldRayOriginKHR
// SPVCH-DAG: BuiltIn WorldRayDirectionKHR
// SPVCH-DAG: BuiltIn RayTmaxKHR
// SPVCH-DAG: BuiltIn HitKindKHR

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

// HLSLAH-DAG: IgnoreHit()
// HLSLAH-DAG: AcceptHitAndEndSearch()
// HLSLAH: [shader("anyhit")]void AnyHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, StructuralTriangleHitAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// SPVAH: OpEntryPoint AnyHitKHR
// SPVAH: OpIgnoreIntersectionKHR
// SPVAH: OpTerminateRayKHR

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

// HLSLIS-DAG: ObjectRayOrigin()
// HLSLIS-DAG: ObjectRayDirection()
// HLSLIS-DAG: PrimitiveIndex()
// HLSLIS-DAG: GeometryIndex()
// HLSLIS-DAG: ReportHit(
// HLSLIS: [shader("intersection")]void Intersection()
// SPVIS: OpEntryPoint IntersectionKHR
// SPVIS-DAG: BuiltIn ObjectRayOriginKHR
// SPVIS-DAG: BuiltIn ObjectRayDirectionKHR
// SPVIS-DAG: BuiltIn PrimitiveId
// SPVIS-DAG: BuiltIn RayGeometryIndexKHR
// SPVIS: OpReportIntersectionKHR
// SPVIS: OpReportIntersectionKHR

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        output[0] = float4(input.worldSpaceOrigin + input.worldSpaceDirection, 1.0);
    }
}

// HLSLMS-DAG: WorldRayOrigin()
// HLSLMS-DAG: WorldRayDirection()
// HLSLMS: [shader("miss")]void Miss(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload)
// SPVMS: OpEntryPoint MissKHR
// SPVMS-DAG: BuiltIn WorldRayOriginKHR
// SPVMS-DAG: BuiltIn WorldRayDirectionKHR
