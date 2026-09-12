//DIAGNOSTIC_TEST:SIMPLE(diag=NOT_FOUND,non-exhaustive): -experimental-feature -entry noEmpty -stage raygeneration -target hlsl -profile sm_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=AMBIGUOUS,non-exhaustive): -experimental-feature -entry ambiguous -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;

struct PayloadWithData
{
    uint value;
}

struct EmptyPayloadA
{}

struct EmptyPayloadB
{}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct PayloadContext<PayloadType> : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadType;
    typealias Record = void;
}

struct Miss<PayloadType> : rt::IMissShader
{
    typealias Context = PayloadContext<PayloadType>;
    void invoke(rt::MissInput<Context> input) {}
}

struct NoEmptySchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss<PayloadWithData>>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct AmbiguousSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss<EmptyPayloadA>, Miss<EmptyPayloadB>>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<NoEmptySchema> noEmptyProgram;
rt::TraceProgramDescriptor<AmbiguousSchema> ambiguousProgram;

[shader("raygeneration")]
void noEmpty()
{
    rt::RayTracer<NoEmptySchema> tracer;
    tracer.trace({}, scene, noEmptyProgram);
//NOT_FOUND:    ^ structural ray-tracing schema has no empty payload
//NOT_FOUND:    ^ schema 'NoEmptySchema' has no concrete empty payload served by a hit group or miss shader
}

[shader("raygeneration")]
void ambiguous()
{
    rt::RayTracer<AmbiguousSchema> tracer;
    tracer.trace({}, scene, ambiguousProgram);
//AMBIGUOUS:    ^ structural ray-tracing schema has multiple empty payloads
//AMBIGUOUS:    ^ schema 'AmbiguousSchema' serves distinct empty payload types 'EmptyPayloadA' and 'EmptyPayloadB'; the three-argument 'trace' overload cannot choose between them
}
