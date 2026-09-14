//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
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
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<HitAttributes>;
    typealias Record = HitRecord;
}

struct TestIntersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
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
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
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
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: {{\[\[}}distance]]
// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing)]]
// METAL: 4.0
// METAL: 2.0
