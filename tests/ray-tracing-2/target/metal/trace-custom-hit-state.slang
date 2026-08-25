//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    float value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct SphereAttributes
{
    float3 normal;
}

struct TestRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<SphereAttributes>;
    typealias Record = TestRecord;
}

struct TestIntersection : rt::IIntersectionShader<HitContext>
{
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        SphereAttributes attributes = { float3(1.0, 2.0, 3.0) };
        input.reportHit(4.0, 17, attributes);
    }
}

struct TestClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = input.attributes.normal.x + input.distance + input.hitKind;
        input.payload.value += input.worldSpaceOrigin.y + input.worldSpaceDirection.z;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = TestIntersection;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: struct ProgramLayout_rayData_0
// METAL: TestPayload_0 payload_{{[0-9]+}};
// METAL: SphereAttributes_0 HitGroup_attributes_{{[0-9]+}};
// METAL: uint customHitKind_{{[0-9]+}};
// METAL: visible_function_table<void(ProgramLayout_rayData_0 thread*, float, uint, float3, float3)>
// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing)]]
// METAL-SAME: ray_data ProgramLayout_rayData_0& rayData_{{[0-9]+}} {{\[\[}}payload]])
// METAL: HitGroup_attributes_{{[0-9]+}} =
// METAL: customHitKind_{{[0-9]+}} = 17U;
// METAL: {{\[\[visible\]\]}} void TestClosestHit_0(
// METAL-SAME: ProgramLayout_rayData_0 thread* rayData_{{[0-9]+}},
// METAL-SAME: float distance_{{[0-9]+}},
// METAL-SAME: uint hitKind_{{[0-9]+}},
// METAL-SAME: float3 worldSpaceOrigin_{{[0-9]+}},
// METAL-SAME: float3 worldSpaceDirection_{{[0-9]+}})
// METAL: HitGroup_attributes_{{[0-9]+}}.normal_{{[0-9]+}}.x
// METAL: customHitKind_{{[0-9]+}}
