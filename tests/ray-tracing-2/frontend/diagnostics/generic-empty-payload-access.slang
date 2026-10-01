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

// The payload type is dependent while this helper is checked. The post-specialization stage
// contract must therefore reject the property operation after Context becomes MissContext.
void copyPayload<Context>(rt::MissInput<Context> input)
    where Context : rt::IPayloadContext
{
    Context.Payload payload = input.payload;
//CHECK:            ^^^^^^^ an empty structural ray-tracing payload has no explicit value
//CHECK:            ^^^^^^^ empty payload type 'EmptyPayload' is represented implicitly; omit the payload argument to 'trace' and do not access 'input.payload'
    input.payload = payload;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input)
    {
        copyPayload<Context>(input);
    }
}
