//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    float value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct SphereAttributes
{
    float3 normal;
}

struct TestRecord
{
    float bias;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<SphereAttributes>;
    typealias Record = TestRecord;
}

struct TestIntersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        SphereAttributes attributes = { float3(1.0, 2.0, 3.0) };
        input.reportHit(4.0 + input.record.bias, 17, attributes);
    }
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = input.attributes.normal.x + input.distance + input.hitKind;
        input.payload.value += input.worldSpaceOrigin.y + input.worldSpaceDirection.z;
        input.payload.value += input.record.bias;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = TestIntersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: struct Schema_rayData_0
// METAL: TestPayload_0 payload_{{[0-9]+}};
// METAL: SphereAttributes_0 HitGroup_attributes_{{[0-9]+}};
// METAL: uint device* descriptorData_{{[0-9]+}};
// METAL: uint customHitKind_{{[0-9]+}};
// METAL: visible_function_table<void(Schema_rayData_0 thread*, float, uint, float3, float3)>
// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing)]]
// METAL-SAME: ray_data Schema_rayData_0& rayData_{{[0-9]+}} {{\[\[}}payload]])
// METAL: HitGroup_attributes_{{[0-9]+}} =
// METAL: customHitKind_{{[0-9]+}} = 17U;
// METAL: {{\[\[visible\]\]}} void __slang_structural_rt_{{[0-9a-f]+}}(
// METAL-SAME: Schema_rayData_0 thread* rayData_{{[0-9]+}},
// METAL-SAME: float distance_{{[0-9]+}},
// METAL-SAME: uint hitKind_{{[0-9]+}},
// METAL-SAME: float3 worldSpaceOrigin_{{[0-9]+}},
// METAL-SAME: float3 worldSpaceDirection_{{[0-9]+}})
// METAL: HitGroup_attributes_{{[0-9]+}}.normal_{{[0-9]+}}.x
// METAL: customHitKind_{{[0-9]+}}
// METAL: TestRecord_0 device*
// METAL: descriptorData_{{[0-9]+}} + int(1)
// METAL: .bias_{{[0-9]+}}
