//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct PayloadA
{
    uint value;
}

struct PayloadB
{
    float value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContextA : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadA;
    typealias Record = void;
}

struct MissContextB : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadB;
    typealias Record = void;
}

struct MissA : rt::IMissShader
{
    typealias Context = MissContextA;

    void invoke(rt::MissInput<Context> input) { input.payload.value = 1; }
}

struct MissB : rt::IMissShader
{
    typealias Context = MissContextB;

    void invoke(rt::MissInput<Context> input) { input.payload.value = 2.0; }
}

struct OnePayloadSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MissA>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct TwoPayloadSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MissA, MissB>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<OnePayloadSchema> onePayloadProgram;
rt::TraceProgramDescriptor<TwoPayloadSchema> twoPayloadProgram;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};

    PayloadA payloadA = {};
    rt::RayTracer<OnePayloadSchema> onePayloadTracer;
    onePayloadTracer.trace(desc, scene, onePayloadProgram, payloadA);

    PayloadB payloadB = {};
    rt::RayTracer<TwoPayloadSchema> twoPayloadTracer;
    twoPayloadTracer.trace(desc, scene, twoPayloadProgram, payloadB);
}

// The opaque source descriptor retains its schema argument until Metal creates one nominal
// physical descriptor type per schema.
// Each schema owns its own argument-buffer type, but both layouts have exactly the same five
// resource roles regardless of the number of payload views.
// METAL-LABEL: struct OnePayloadSchema_resources{{.*}}
// METAL: intersectionFunctions_0;
// METAL: missFunctions_0;
// METAL: closestHitFunctions_0;
// METAL: callableFunctions_0;
// METAL: records_0;
// METAL-LABEL: struct TwoPayloadSchema_resources{{.*}}
// METAL: intersectionFunctions_1;
// METAL: missFunctions_1;
// METAL: closestHitFunctions_1;
// METAL: callableFunctions_1;
// METAL: records_1;
// METAL-LABEL: struct KernelContext_0
// METAL: OnePayloadSchema_resources{{.*}} constant* onePayloadProgram_0;
// METAL: TwoPayloadSchema_resources{{.*}} constant* twoPayloadProgram_0;
