//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;

struct EmptyPayload
{}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    EmptyPayload payload = {};
    tracer.trace<EmptyPayload>({}, scene, program, payload);
//CHECK:                                           ^^^^^^^ an empty structural ray-tracing payload has no explicit value
//CHECK:                                           ^^^^^^^ empty payload type 'EmptyPayload' is represented implicitly; omit the payload argument to 'trace' and do not access 'input.payload'
}
