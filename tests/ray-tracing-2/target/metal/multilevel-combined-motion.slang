//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct MotionTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::PrimitiveAndInstanceMotion;
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

rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.75;
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: metal::raytracing::acceleration_structure<metal::raytracing::instancing, metal::raytracing::primitive_motion, metal::raytracing::instance_motion> scene_{{[0-9]+}}
// METAL: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::primitive_motion, metal::raytracing::instance_motion, metal::raytracing::max_levels<3>> _slang_intersector;
// METAL: metal::raytracing::intersection_result<metal::raytracing::instancing, metal::raytracing::primitive_motion, metal::raytracing::instance_motion, metal::raytracing::max_levels<3>> _slang_result
// METAL: scene_{{[0-9]+}}, {{.*}}.instanceMask_0, {{.*}}.time_0);
