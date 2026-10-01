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

struct Attributes
{
    float value;
}

struct TestRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = TestRecord;
}

struct TestIntersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        float value = input.objectSpaceRay.origin.x + input.objectSpaceRay.direction.y;
        value += input.primitiveIndex + input.geometryIndex;
        value += input.worldSpaceOrigin.z + input.worldSpaceDirection.z;
        Attributes attributes = { value };
        input.reportHit(1.0, attributes);
    }
}

struct TestAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.value = input.worldSpaceOrigin.x + input.worldSpaceDirection.y;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = TestAnyHit;
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
