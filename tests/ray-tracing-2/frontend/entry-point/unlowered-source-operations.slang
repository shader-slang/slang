// Trace and callable dispatch retain their opaque structural descriptor until target adapter
// lowering chooses its representation.
//DIAGNOSTIC_TEST:SIMPLE(diag=TRACE): -experimental-feature -entry traceMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=TRACE): -experimental-feature -entry tracePayloadMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CALL): -experimental-feature -entry callMain -stage raygeneration -target hlsl


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
/*TRACE:
                                   ^^^^^^^^^^ structural ray-tracing program descriptor must be lowered before target code generation
                                   ^^^^^^^^^^ target adapter lowering must choose the descriptor's physical representation
*/
/*CALL:
                                   ^^^^^^^^^^ structural ray-tracing program descriptor must be lowered before target code generation
                                   ^^^^^^^^^^ target adapter lowering must choose the descriptor's physical representation
*/

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
