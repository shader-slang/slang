//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct ServedTag {}
struct UnservedTag {}

// The tag is intentionally absent from storage. These two specializations therefore have the
// same native payload shape, but they remain different source-semantic payload types.
struct PayloadValue<Tag>
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadValue<ServedTag>;
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
    PayloadValue<UnservedTag> payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ structural ray-tracing schema does not serve this payload
//CHECK:        ^ schema 'Schema' has no hit group or miss shader for payload type 'PayloadValue'
}
