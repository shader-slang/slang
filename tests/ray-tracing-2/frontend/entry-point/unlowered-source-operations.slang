// Native trace calls retain their opaque structural descriptor until target adapter lowering
// chooses its representation. Callable dispatch also retains its unfinished operation marker.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=TRACE): -experimental-feature -entry traceMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PAYLOAD): -experimental-feature -entry tracePayloadMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -experimental-feature -entry callMain -stage raygeneration -target hlsl

// TRACE-NOT: structural ray-tracing operation 'trace'
// TRACE: structural ray-tracing program descriptor must be lowered before target code generation
// TRACE-NOT: structural ray-tracing operation 'trace'
// PAYLOAD-NOT: structural ray-tracing operation 'trace'
// PAYLOAD: structural ray-tracing program descriptor must be lowered before target code generation
// PAYLOAD-NOT: structural ray-tracing operation 'trace'
// CALL: structural ray-tracing operation 'callShader' reachable from this entry point must be lowered

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct CallableData
{
    uint value;
}

struct Payload
{
    uint value;
}

struct EmptyPayload
{}

struct EmptyMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::EmptyPayload;
    typealias Record = void;
}

struct PayloadMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct EmptyMiss : rt::IMissShader
{
    typealias Context = EmptyMissContext;

    static void invoke(in rt::MissInput<Context> input) {}
}

struct PayloadMiss : rt::IMissShader
{
    typealias Context = PayloadMissContext;

    static void invoke(in rt::MissInput<Context> input) {}
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

    static void invoke(in rt::CallableInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::MissShaderList<EmptyMiss, PayloadMiss>;
    typealias CallableShaders = rt::CallableShaderList<Callable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> descriptor;

[shader("raygeneration")]
void traceMain()
{
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, descriptor);
}

[shader("raygeneration")]
void tracePayloadMain()
{
    rt::RayTracer<Schema> tracer;
    Payload payload = {};
    tracer.trace({}, scene, descriptor, payload);
}

[shader("raygeneration")]
void callMain()
{
    rt::RayTracer<Schema> tracer;
    CallableData data = {};
    tracer.callShader<CallableContext>(0, descriptor, data);
}
