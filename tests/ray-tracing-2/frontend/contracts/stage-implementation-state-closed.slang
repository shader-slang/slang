// A stage selected only through a closed schema must obey the same stateless contract as a struct
// selected directly with `-entry StructName`. The compiler creates the stage receiver, so there is
// no source instance from which it could initialize even an inherited `state` field.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -no-codegen

#pragma warning(disable: 30816)

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

struct StatefulStageBase
{
    uint state;
//CHECK: ^^^^^ stateful structural ray-tracing stage
//CHECK: ^^^^^ instance field 'state' is not allowed because structural ray-tracing stage implementations are compiler-created
}

struct StatefulClosestHit : StatefulStageBase,
                            rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<Context> input) {}
}

// Sharing one invalid base between stages must not duplicate the diagnostic on `state`.
struct SecondStatefulClosestHit : StatefulStageBase,
                                  rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<Context> input) {}
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = StatefulClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct SecondHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SecondStatefulClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup, SecondHitGroup>;
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
