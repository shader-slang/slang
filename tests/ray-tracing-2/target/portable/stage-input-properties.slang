//TEST:SIMPLE(filecheck=HLSLCH): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=HLSLAH): -experimental-feature -entry AnyHit -stage anyhit -target hlsl
//TEST:SIMPLE(filecheck=HLSLIS): -experimental-feature -entry Intersection -stage intersection -target hlsl
//TEST:SIMPLE(filecheck=HLSLMS): -experimental-feature -entry Miss -stage miss -target hlsl
//TEST:SIMPLE(filecheck=SPVCH): -experimental-feature -entry ClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVAH): -experimental-feature -entry AnyHit -stage anyhit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVIS): -experimental-feature -entry Intersection -stage intersection -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVMS): -experimental-feature -entry Miss -stage miss -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSLLAYOUT): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPVLAYOUT): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=DXILLAYOUT): -experimental-feature -entry main -stage raygeneration -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=METALLAYOUT): -experimental-feature -entry main -stage raygeneration -target metal -profile metallib_2_4

import slang.raytracing;

struct TestPayload
{
    uint value;
}
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
        let objectRay = input.objectSpaceRay;
        output[0] = float4(
            input.worldSpaceOrigin.x + input.worldSpaceDirection.x + objectRay.origin.x +
                objectRay.direction.x,
            input.distance + input.minDistance,
            float(input.hitKind + input.rayFlags) + input.objectToWorld[0][0] +
                input.worldToObject[0][0],
            float(input.primitiveIndex + input.geometryIndex + input.instanceIndex +
                  input.instanceID + input.dispatchRaysIndex.x +
                  input.dispatchRaysDimensions.x));
    }
}

// HLSLCH-DAG: WorldRayOrigin()
// HLSLCH-DAG: WorldRayDirection()
// HLSLCH-DAG: RayTCurrent()
// HLSLCH-DAG: HitKind()
// HLSLCH-DAG: RayTMin()
// HLSLCH-DAG: RayFlags()
// HLSLCH-DAG: PrimitiveIndex()
// HLSLCH-DAG: GeometryIndex()
// HLSLCH-DAG: InstanceIndex()
// HLSLCH-DAG: InstanceID()
// HLSLCH-DAG: ObjectRayOrigin()
// HLSLCH-DAG: ObjectRayDirection()
// HLSLCH-DAG: ObjectToWorld4x3()
// HLSLCH-DAG: WorldToObject4x3()
// HLSLCH-DAG: DispatchRaysIndex()
// HLSLCH-DAG: DispatchRaysDimensions()
// HLSLCH: [shader("closesthit")]void ClosestHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, StructuralTriangleHitAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// SPVCH: OpEntryPoint ClosestHitKHR
// SPVCH-DAG: BuiltIn WorldRayOriginKHR
// SPVCH-DAG: BuiltIn WorldRayDirectionKHR
// SPVCH-DAG: BuiltIn RayTmaxKHR
// SPVCH-DAG: BuiltIn HitKindKHR
// SPVCH-DAG: BuiltIn RayTminKHR
// SPVCH-DAG: BuiltIn IncomingRayFlagsKHR
// SPVCH-DAG: BuiltIn PrimitiveId
// SPVCH-DAG: BuiltIn RayGeometryIndexKHR
// SPVCH-DAG: BuiltIn InstanceId
// SPVCH-DAG: BuiltIn InstanceCustomIndexKHR
// SPVCH-DAG: BuiltIn ObjectRayOriginKHR
// SPVCH-DAG: BuiltIn ObjectRayDirectionKHR
// SPVCH-DAG: BuiltIn ObjectToWorldKHR
// SPVCH-DAG: BuiltIn WorldToObjectKHR
// SPVCH-DAG: BuiltIn LaunchIdKHR
// SPVCH-DAG: BuiltIn LaunchSizeKHR

struct AnyHit : rt::IAnyHitShader<TriangleContext>
{
    void invoke(rt::AnyHitInput<TriangleContext> input)
    {
        let objectRay = input.objectSpaceRay;
        if (input.worldSpaceOrigin.x + input.worldSpaceDirection.x + input.distance +
                input.minDistance + float(input.rayFlags + input.primitiveIndex +
                                          input.geometryIndex + input.instanceIndex +
                                          input.instanceID) +
                objectRay.origin.x + objectRay.direction.x + input.objectToWorld[0][0] +
                input.worldToObject[0][0] + float(input.dispatchRaysIndex.x) +
                float(input.dispatchRaysDimensions.x) >
            0.0)
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
                           float(input.geometryIndex + input.instanceIndex + input.instanceID +
                                 input.rayFlags) +
                           input.minDistance + input.objectToWorld[0][0] +
                           input.worldToObject[0][0] + float(input.dispatchRaysIndex.x) +
                           float(input.dispatchRaysDimensions.x);
        if (!input.reportHit(ray.tMin + input.objectToWorld[0][0], attributes))
            input.reportHit(ray.tMax, 1, attributes);
    }
}

// HLSLIS-DAG: ObjectRayOrigin()
// HLSLIS-DAG: ObjectRayDirection()
// HLSLIS-DAG: PrimitiveIndex()
// HLSLIS-DAG: GeometryIndex()
// HLSLIS-DAG: ObjectToWorld4x3()
// HLSLIS-DAG: WorldToObject4x3()
// HLSLIS-DAG: ReportHit(
// HLSLIS: [shader("intersection")]void Intersection()
// SPVIS: OpEntryPoint IntersectionKHR
// SPVIS-DAG: BuiltIn ObjectRayOriginKHR
// SPVIS-DAG: BuiltIn ObjectRayDirectionKHR
// SPVIS-DAG: BuiltIn PrimitiveId
// SPVIS-DAG: BuiltIn RayGeometryIndexKHR
// SPVIS-DAG: BuiltIn ObjectToWorldKHR
// SPVIS-DAG: BuiltIn WorldToObjectKHR
// SPVIS: OpReportIntersectionKHR
// SPVIS: OpReportIntersectionKHR

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        output[0] = float4(
            input.worldSpaceOrigin + input.worldSpaceDirection,
            input.minDistance + input.distance + float(input.rayFlags) +
                float(input.dispatchRaysIndex.x + input.dispatchRaysDimensions.x));
    }
}

// HLSLMS-DAG: WorldRayOrigin()
// HLSLMS-DAG: WorldRayDirection()
// HLSLMS: [shader("miss")]void Miss(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload)
// SPVMS: OpEntryPoint MissKHR
// SPVMS-DAG: BuiltIn WorldRayOriginKHR
// SPVMS-DAG: BuiltIn WorldRayDirectionKHR
// SPVMS-DAG: BuiltIn LaunchIdKHR
// SPVMS-DAG: BuiltIn LaunchSizeKHR

typealias ClosestHitStage = ClosestHit;
typealias AnyHitStage = AnyHit;
typealias IntersectionStage = Intersection;
typealias MissStage = Miss;

struct TriangleHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = TriangleContext;
    typealias ClosestHit = ClosestHitStage;
    typealias AnyHit = AnyHitStage;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct ProceduralHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<1>;
    typealias Context = ProceduralContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = IntersectionStage;
}

struct TestMissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = MissStage;
}

struct TestProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups =
        rt::HitGroupList<TestTraceContext, TriangleHitGroup, ProceduralHitGroup>;
    typealias MissGroups = rt::MissGroupList<TestTraceContext, TestMissGroup>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<TestProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestProgramLayout> tracer;
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    tracer.trace(desc, scene, program, payload);
}

// HLSLLAYOUT-DAG: WorldRayOrigin()
// HLSLLAYOUT-DAG: IgnoreHit()
// HLSLLAYOUT-DAG: ReportHit(
// HLSLLAYOUT-DAG: [shader("closesthit")]void ClosestHit(
// HLSLLAYOUT-DAG: [shader("anyhit")]void AnyHit(
// HLSLLAYOUT-DAG: [shader("intersection")]void Intersection()
// HLSLLAYOUT-DAG: [shader("miss")]void Miss(

// SPVLAYOUT-DAG: OpEntryPoint RayGenerationKHR %main "main"
// SPVLAYOUT-DAG: OpEntryPoint ClosestHitKHR %ClosestHit "ClosestHit"
// SPVLAYOUT-DAG: OpEntryPoint AnyHitKHR %AnyHit "AnyHit"
// SPVLAYOUT-DAG: OpEntryPoint IntersectionKHR %Intersection "Intersection"
// SPVLAYOUT-DAG: OpEntryPoint MissKHR %Miss "Miss"
// SPVLAYOUT-DAG: OpIgnoreIntersectionKHR
// SPVLAYOUT-DAG: OpReportIntersectionKHR

// DXILLAYOUT-DAG: ClosestHit
// DXILLAYOUT-DAG: AnyHit
// DXILLAYOUT-DAG: Intersection
// DXILLAYOUT-DAG: Miss

// METALLAYOUT-DAG: {{\[\[visible\]\]}} void ClosestHit_
// METALLAYOUT-DAG: {{\[\[intersection\(triangle,}}
// METALLAYOUT-DAG: uint primitiveIndex_{{[0-9]+}} {{\[\[primitive_id\]\]}}
// METALLAYOUT-DAG: uint geometryIndex_{{[0-9]+}} {{\[\[geometry_id\]\]}}
// METALLAYOUT-DAG: uint instanceIndex_{{[0-9]+}} {{\[\[instance_id\]\]}}
// METALLAYOUT-DAG: uint instanceID_{{[0-9]+}} {{\[\[user_instance_id\]\]}}
// METALLAYOUT-DAG: {{\[\[origin\]\]}}
// METALLAYOUT-DAG: {{\[\[direction\]\]}}
// METALLAYOUT-DAG: {{\[\[object_to_world_transform\]\]}}
// METALLAYOUT-DAG: {{\[\[world_to_object_transform\]\]}}
// METALLAYOUT-DAG: {{\[\[intersection\(bounding_box,}}
// METALLAYOUT-DAG: _slang_result.primitive_id
// METALLAYOUT-DAG: _slang_result.geometry_id
// METALLAYOUT-DAG: _slang_result.instance_id
// METALLAYOUT-DAG: _slang_result.user_instance_id
// METALLAYOUT-DAG: _slang_result.object_to_world_transform
// METALLAYOUT-DAG: _slang_result.world_to_object_transform
// METALLAYOUT-DAG: uint3 dispatchRaysIndex_{{[0-9]+}} {{\[\[thread_position_in_grid\]\]}}
// METALLAYOUT-DAG: uint3 dispatchRaysDimensions_{{[0-9]+}} {{\[\[threads_per_grid\]\]}}
