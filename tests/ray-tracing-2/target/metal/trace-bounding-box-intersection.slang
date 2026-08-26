//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitAttributes
{
    float3 normal;
}

struct HitRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<HitAttributes>;
    typealias Record = HitRecord;
}

struct TestIntersection : rt::IIntersectionShader<HitContext>
{
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        HitAttributes farAttributes = { float3(0.0, 0.0, 1.0) };
        input.reportHit(4.0, 4, farAttributes);

        HitAttributes nearAttributes = { float3(0.0, 1.0, 0.0) };
        input.reportHit(2.0, 2, nearAttributes);
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
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
    rt::RayTraversalDesc desc;
    desc.ray.origin = float3(0.0, 0.0, 0.0);
    desc.ray.tMin = 0.001;
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 1000.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 1;
    desc.missIndex = 0;

    TestPayload payload = { 0 };
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: {{\[\[}}distance]]
// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing)]]
// METAL: 4.0
// METAL: 2.0
