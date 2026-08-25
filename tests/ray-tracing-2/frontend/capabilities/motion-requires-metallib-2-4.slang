//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry main -stage raygeneration -target metal -profile metallib_2_3 -restrictive-capability-check

import slang.raytracing;

struct TestPayload {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::PrimitiveMotion;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups<TestTraceContext>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
/*CHECK:
     ^^^^ entry point uses capabilities not in specified profile
*/
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
}
