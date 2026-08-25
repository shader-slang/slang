//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct MotionTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<1>;
    typealias Motion = rt::PrimitiveMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MotionTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct TestClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = MotionTraceContext;
    typealias HitGroups = rt::HitGroupList<MotionTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<MotionTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<MotionTraceContext>;
}

rt::MultiLevelAccelerationStructure<1> scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.5;
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: metal::raytracing::acceleration_structure<metal::raytracing::primitive_motion> scene_{{[0-9]+}}
// METAL: metal::raytracing::intersector<metal::raytracing::primitive_motion> _slang_intersector;
// METAL: metal::raytracing::intersection_result<metal::raytracing::primitive_motion> _slang_result
// METAL: scene_{{[0-9]+}}, {{.*}}.time_0);
