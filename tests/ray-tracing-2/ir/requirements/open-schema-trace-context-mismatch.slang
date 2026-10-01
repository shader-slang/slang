//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct Payload
{
    uint value;
}

struct SchemaTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct EntryTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = EntryTraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

interface IHitTag : rt::IHitGroup {}

struct LinkedHitGroup : IHitTag
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = SchemaTraceContext;
    typealias HitGroups = rt::OpenHitGroups<IHitTag>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    Payload payload = {};
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ structural ray-tracing entry uses a different trace context
//CHECK:        ^ entry 'LinkedHitGroup' uses trace context 'EntryTraceContext', but schema 'Schema' requires 'SchemaTraceContext'
}
