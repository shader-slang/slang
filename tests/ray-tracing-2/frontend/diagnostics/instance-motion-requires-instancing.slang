//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<1>;
    typealias Motion = rt::InstanceMotion;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups<TestTraceContext>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::MultiLevelAccelerationStructure<1> scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
//CHECK:        ^ structural ray-tracing instance motion requires instancing
//CHECK:        ^ instance motion cannot be used with a direct primitive acceleration structure
}
