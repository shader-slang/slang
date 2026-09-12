//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct MotionTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<1>;
    typealias Motion = rt::PrimitiveMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MotionTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = MotionTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::MultiLevelAccelerationStructure<1> scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.5;
    TestPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: metal::raytracing::acceleration_structure<metal::raytracing::primitive_motion> scene_{{[0-9]+}}
// METAL: metal::raytracing::intersector<metal::raytracing::primitive_motion> _slang_intersector;
// METAL: metal::raytracing::intersection_result<metal::raytracing::primitive_motion> _slang_result
// METAL: scene_{{[0-9]+}}, {{.*}}.time_0);
