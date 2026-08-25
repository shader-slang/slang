//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal -profile metallib_2_4 -restrictive-capability-check

import slang.raytracing;

struct TestPayload {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups<TestTraceContext>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
//CHECK:        ^ structural ray-tracing multilevel traversal requires MetalLib 3.1
//CHECK:        ^ multi-level acceleration structures with two or more levels require the 'metallib_3_1' capability
}
