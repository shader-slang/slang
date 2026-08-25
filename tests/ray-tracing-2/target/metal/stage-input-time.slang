//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal -profile metallib_2_4

import slang.raytracing;

struct TestPayload
{
    float value;
}

struct Attributes
{
    float value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::PrimitiveAndInstanceMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = void;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = void;
}

struct TestClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = input.time + input.attributes.value;
    }
}

struct TestAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.value += input.time;
        if (input.time < 0.0)
            input.ignoreHit();
    }
}

struct TestIntersection : rt::IIntersectionShader<HitContext>
{
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        Attributes attributes = {input.time};
        input.reportHit(1.0, attributes);
    }
}

struct TestMiss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.value = input.time;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = TestIntersection;
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = TestMiss;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::MissGroupList<TestTraceContext, MissGroup>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.375;
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL-DAG: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::primitive_motion, metal::raytracing::instance_motion> _slang_intersector;
// METAL-DAG: float rayTime_{{[0-9]+}};
// METAL-DAG: (&{{rayData_[0-9]+}})->rayTime_{{[0-9]+}} = {{desc_[0-9]+}}.time_{{[0-9]+}};
// METAL-DAG: {{\[\[intersection\(bounding_box,}}
// METAL-DAG: {{\[\[visible\]\]}} void TestClosestHit_0
// METAL-DAG: {{\[\[visible\]\]}} void TestMiss_0
// METAL-NOT: structuralRayTracingGetRayTime
