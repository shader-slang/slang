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
    typealias Motion = rt::PrimitiveMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
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
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> program;

void traceFromHelper(
    rt::AccelerationStructure accelerationStructure,
    rt::RayTraversalDesc desc,
    inout TestPayload payload)
{
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, accelerationStructure, program, payload);
}

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.5;
    TestPayload payload = {};
    traceFromHelper(scene, desc, payload);
}

// METAL: metal::raytracing::acceleration_structure<metal::raytracing::instancing, metal::raytracing::primitive_motion> scene_{{[0-9]+}}
// METAL: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::primitive_motion> _slang_intersector;
