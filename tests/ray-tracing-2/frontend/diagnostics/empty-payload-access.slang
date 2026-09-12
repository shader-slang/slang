//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry Miss -stage miss -target hlsl -profile sm_6_5

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
    void invoke(rt::MissInput<Context> input)
    {
        EmptyPayload payload = input.payload;
//CHECK:                             ^^^^^^^ an empty structural ray-tracing payload has no explicit value
//CHECK:                             ^^^^^^^ empty payload type 'EmptyPayload' is represented implicitly; omit the payload argument to 'trace' and do not access 'input.payload'
    }
}
