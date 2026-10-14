//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

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

// Each listed implementation receives one function index. Repeating a type in any section is
// invalid; the host can instead reference the same listed implementation from multiple SBT records.
struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup, HitGroup>;
//CHECK:      ^^^^^^^^^ duplicate structural ray-tracing entry
//CHECK:      ^^^^^^^^^ hit-group entry 'HitGroup' is listed more than once in schema 'Schema'
    typealias MissShaders = rt::MissShaderList<Miss, Miss>;
//CHECK:      ^^^^^^^^^^^ duplicate structural ray-tracing entry
//CHECK:      ^^^^^^^^^^^ miss-shader entry 'Miss' is listed more than once in schema 'Schema'
    typealias CallableShaders = rt::CallableShaderList<Callable, Callable>;
//CHECK:      ^^^^^^^^^^^^^^^ duplicate structural ray-tracing entry
//CHECK:      ^^^^^^^^^^^^^^^ callable-shader entry 'Callable' is listed more than once in schema 'Schema'
}
