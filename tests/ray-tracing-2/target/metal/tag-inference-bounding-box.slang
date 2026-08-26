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

struct Attributes
{
    float value;
}

struct TestRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = TestRecord;
}

struct TestIntersection : rt::IIntersectionShader<HitContext>
{
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        float value = input.objectSpaceRay.origin.x + input.objectSpaceRay.direction.y;
        value += input.primitiveIndex + input.geometryIndex;
        value += input.worldSpaceOrigin.z + input.worldSpaceDirection.z;
        Attributes attributes = { value };
        input.reportHit(1.0, attributes);
    }
}

struct TestAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.value = input.worldSpaceOrigin.x + input.worldSpaceDirection.y;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = TestIntersection;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing, metal::raytracing::world_space_data)]]
// METAL-SAME: float3 objectSpaceOrigin_{{[0-9]+}} {{\[\[}}origin]]
// METAL-SAME: float3 objectSpaceDirection_{{[0-9]+}} {{\[\[}}direction]]
// METAL-SAME: uint primitiveIndex_{{[0-9]+}} {{\[\[}}primitive_id]]
// METAL-SAME: uint geometryIndex_{{[0-9]+}} {{\[\[}}geometry_id]]
// METAL-SAME: float3 worldSpaceOrigin_{{[0-9]+}} {{\[\[}}world_space_origin]]
// METAL-SAME: float3 worldSpaceDirection_{{[0-9]+}} {{\[\[}}world_space_direction]]
// METAL-SAME: bool opaque_{{[0-9]+}} {{\[\[}}opaque]]
// METAL: if(opaque_
// METAL: break;
// METAL: rayDataStorage_{{[0-9]+}})->payload_{{[0-9]+}})->value_{{[0-9]+}} = worldSpaceOrigin_
