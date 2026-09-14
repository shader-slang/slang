//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal -profile metallib_2_4 -restrictive-capability-check

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
//CHECK:        ^ structural ray-tracing multilevel traversal requires MetalLib 3.1
//CHECK:        ^ multi-level acceleration structures with two or more levels require the 'metallib_3_1' capability
}
