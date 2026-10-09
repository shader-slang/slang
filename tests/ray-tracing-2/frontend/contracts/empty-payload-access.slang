//TEST:SIMPLE: -experimental-feature -entry Miss -stage miss -target hlsl -profile sm_6_5 -no-codegen

// Empty payloads are ordinary source values; target legalization owns their physical storage.
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
        input.payload = payload;
    }
}
