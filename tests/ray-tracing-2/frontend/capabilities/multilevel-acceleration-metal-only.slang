//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<2>;
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

rt::MultiLevelAccelerationStructure<2> scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
/*CHECK:
     ^^^^ unavailable features in entry point
*/
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}
