// An any-hit stage selected through an explicit hit-group list is compiler-created and cannot
// carry instance storage, just like a stage selected directly with `-entry StructName`.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -no-codegen

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

struct StatefulAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    bool alphaTestEnabled;
//CHECK: ^^^^^^^^^^^^^^^^ stateful structural ray-tracing stage
//CHECK: ^^^^^^^^^^^^^^^^ instance field 'alphaTestEnabled' is not allowed because structural ray-tracing stage implementations are compiler-created

    void invoke(rt::AnyHitInput<Context> input) {}
}

interface IHitGroupContract : rt::IHitGroup {}

struct HitGroup : IHitGroupContract
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = StatefulAnyHit;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, program, payload);
}
