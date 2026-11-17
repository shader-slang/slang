// A stage reached only through a schema gets its implementation from a generic extension. The
// schema's hit-group list drives the conformance lookup for the entry, which applies the
// extension and its deferred user-defined-struct rule.
//TEST:SIMPLE: -experimental-feature -no-codegen
//TEST:SIMPLE: -experimental-feature -entry SchemaHit -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry SchemaHit.invoke -stage closesthit -no-codegen

import slang.raytracing.portable;

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

interface IMarker {}

__generic<T : IMarker>
extension T : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static void invoke(in rt::ClosestHitInput<Context> input) {}
}

struct SchemaHit : IMarker
{
    uint state;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SchemaHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Program : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

[numthreads(1, 1, 1)]
void main() {}
