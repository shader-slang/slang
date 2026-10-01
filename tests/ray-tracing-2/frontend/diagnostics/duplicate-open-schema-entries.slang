//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -no-codegen

import slang.raytracing;

struct Payload
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
    typealias Payload = ::Payload;
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
    typealias Payload = ::Payload;
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

interface IHitTag : rt::IHitGroup {}
interface IMissTag : rt::IMissShader {}
interface ICallableTag : rt::ICallableShader {}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::OpenHitGroups<IHitTag, HitGroup, HitGroup>;
    typealias MissShaders = rt::OpenMissShaders<IMissTag, Miss, Miss>;
    typealias CallableShaders = rt::OpenCallableShaders<ICallableTag, Callable, Callable>;
}

// Open sections use their explicit pack as the ordered prefix of the linked implementation set.
// Repeating a type in that prefix is invalid even though a listed type that also conforms to its
// tag is valid and will be unioned only once during link completion.
//CHECK-COUNT-3: error[E38074]: duplicate structural ray-tracing entry
//CHECK-DAG: hit-group entry 'HitGroup' is listed more than once in schema 'Schema'
//CHECK-DAG: miss-shader entry 'Miss' is listed more than once in schema 'Schema'
//CHECK-DAG: callable-shader entry 'Callable' is listed more than once in schema 'Schema'
