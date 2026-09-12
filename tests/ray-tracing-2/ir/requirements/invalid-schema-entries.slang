//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct ServedPayload
{
    uint value;
}
struct UnservedPayload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ServedPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ServedPayload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = uint;
    typealias Record = void;
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup, HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss, Miss>;
    typealias CallableShaders = rt::CallableShaderList<Callable, Callable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    UnservedPayload payload = {};
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ duplicate structural ray-tracing entry
//CHECK:        ^ callable-shader entry 'Callable' is listed more than once in schema 'Schema'
//CHECK:        ^ duplicate structural ray-tracing entry
//CHECK:        ^ miss-shader entry 'Miss' is listed more than once in schema 'Schema'
//CHECK:        ^ duplicate structural ray-tracing entry
//CHECK:        ^ hit-group entry 'HitGroup' is listed more than once in schema 'Schema'
//CHECK:        ^ structural ray-tracing schema does not serve this payload
//CHECK:        ^ schema 'Schema' has no hit group or miss shader for payload type 'UnservedPayload'
}
