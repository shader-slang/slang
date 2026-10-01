//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -no-codegen

import slang.raytracing;

struct ServedPayload
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

// A list names code implementations, not host-created SBT records. Repeating one type would give
// the same implementation two dense function indices, so the schema contract rejects it before a
// trace call is lowered.
//CHECK-COUNT-3: error[E38074]: duplicate structural ray-tracing entry
//CHECK-DAG: hit-group entry 'HitGroup' is listed more than once in schema 'Schema'
//CHECK-DAG: miss-shader entry 'Miss' is listed more than once in schema 'Schema'
//CHECK-DAG: callable-shader entry 'Callable' is listed more than once in schema 'Schema'
