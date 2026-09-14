//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct StaticTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MotionTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::InstanceMotion;
}

struct StaticMissContext : rt::IPayloadContext
{
    typealias TraceContext = StaticTraceContext;
    typealias Payload = TestPayload;
    typealias Record = void;
}

struct MotionMissContext : rt::IPayloadContext
{
    typealias TraceContext = MotionTraceContext;
    typealias Payload = TestPayload;
    typealias Record = void;
}

struct StaticMiss : rt::IMissShader
{
    typealias Context = StaticMissContext;
    void invoke(rt::MissInput<StaticMissContext> input)
    {
        input.payload.value = 1;
    }
}

struct MotionMiss : rt::IMissShader
{
    typealias Context = MotionMissContext;
    void invoke(rt::MissInput<MotionMissContext> input)
    {
        input.payload.value = 2;
    }
}

struct StaticSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = StaticTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<StaticMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct MotionSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = MotionTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MotionMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct SceneParameters
{
    rt::AccelerationStructure scene;
}

ParameterBlock<SceneParameters> sceneParameters;
rt::TraceProgramDescriptor<StaticSchema> staticProgram;
rt::TraceProgramDescriptor<MotionSchema> motionProgram;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};

    rt::RayTracer<StaticSchema> staticTracer;
    staticTracer.trace(desc, sceneParameters.scene, staticProgram, payload);

    // One resource cannot have both Metal native acceleration-structure types. Diagnose the
    // second trace at compile time instead of emitting a field with an ABI chosen by traversal
    // order.
    rt::RayTracer<MotionSchema> motionTracer;
    motionTracer.trace(desc, sceneParameters.scene, motionProgram, payload);
//CHECK: conflicting structural ray-tracing acceleration-structure requirements
//CHECK: the same acceleration-structure value is used by trace contexts with incompatible topology or motion requirements
}
