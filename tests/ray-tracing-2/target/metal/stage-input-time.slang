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
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::PrimitiveAndInstanceMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = void;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Record = void;
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = input.time + input.attributes.value;
    }
}

struct TestAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.value += input.time;
        if (input.time < 0.0)
            input.ignoreHit();
    }
}

struct TestIntersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        Attributes attributes = {input.time};
        input.reportHit(1.0, attributes);
    }
}

struct TestMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.value = input.time;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = TestIntersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<TestMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.375;
    TestPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL-DAG: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::primitive_motion, metal::raytracing::instance_motion> _slang_intersector;
// METAL-DAG: float rayTime_{{[0-9]+}};
// METAL-DAG: (&{{rayData_[0-9]+}})->rayTime_{{[0-9]+}} = {{desc_[0-9]+}}.time_{{[0-9]+}};
// METAL-DAG: {{\[\[intersection\(bounding_box,}}
// METAL-DAG: {{\[\[visible\]\]}} void __slang_structural_rt_
// METAL-DAG: {{\[\[visible\]\]}} void __slang_structural_rt_
// METAL-NOT: structuralRayTracingGetRayTime
