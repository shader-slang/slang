//DIAGNOSTIC_TEST:SIMPLE(diag=NOT_FOUND,non-exhaustive): -experimental-feature -entry noEmpty -stage raygeneration -target hlsl -profile sm_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=AMBIGUOUS,non-exhaustive): -experimental-feature -entry ambiguous -stage raygeneration -target hlsl -profile sm_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT,non-exhaustive): -experimental-feature -entry explicitPayload -stage raygeneration -target hlsl -profile sm_6_5
//DIAGNOSTIC_TEST:SIMPLE(diag=CALL_ONLY,non-exhaustive): -experimental-feature -entry callOnly -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;

struct PayloadWithData
{
    uint value;
}

struct EmptyPayloadA
{
}

struct EmptyPayloadB
{
}

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

struct CallableData
{
    uint value;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = void;
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input) {}
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
    typealias MissShaders =
        rt::MissShaderList<Miss<EmptyPayloadA>, Miss<EmptyPayloadB>, Miss<PayloadWithData>>;
    typealias CallableShaders = rt::CallableShaderList<Callable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<NoEmptySchema> noEmptyProgram;
rt::TraceProgramDescriptor<AmbiguousSchema> ambiguousProgram;

[shader("raygeneration")]
void noEmpty()
{
    rt::RayTracer<NoEmptySchema> tracer;
    tracer.trace({}, scene, noEmptyProgram);
    // NOT_FOUND:    ^ structural ray-tracing schema has no empty payload
    // NOT_FOUND:    ^ schema 'NoEmptySchema' has no concrete empty payload served by a hit group or miss shader
}

[shader("raygeneration")]
void ambiguous()
{
    rt::RayTracer<AmbiguousSchema> tracer;
    tracer.trace({}, scene, ambiguousProgram);
    // AMBIGUOUS:    ^ structural ray-tracing schema has multiple empty payloads
    // AMBIGUOUS:    ^ schema 'AmbiguousSchema' serves distinct empty payload types 'EmptyPayloadA' and 'EmptyPayloadB'; a schema may serve at most one empty payload type
}

// Selecting a non-empty payload does not hide an invalid schema-wide empty-payload set.
[shader("raygeneration")]
void explicitPayload()
{
    rt::RayTracer<AmbiguousSchema> tracer;
    PayloadWithData payload = {};
    tracer.trace<PayloadWithData>({}, scene, ambiguousProgram, payload);
}

// EXPLICIT: structural ray-tracing schema has multiple empty payloads
// EXPLICIT: a schema may serve at most one empty payload type

// A callable dispatch does not select a ray payload, but it still activates the complete schema.
// The schema-wide empty-payload invariant must therefore be checked for a CallShader owner too.
[shader("raygeneration")]
void callOnly()
{
    rt::RayTracer<AmbiguousSchema> tracer;
    CallableData data = {};
    tracer.callShader<CallableContext>(0, ambiguousProgram, data);
}

// CALL_ONLY: structural ray-tracing schema has multiple empty payloads
// CALL_ONLY: a schema may serve at most one empty payload type
