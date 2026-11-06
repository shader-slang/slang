// Selecting an any-hit stage through a hit group does not construct its ordinary object fields.
//TEST:SIMPLE: -experimental-feature -entry main -stage raygeneration -no-codegen

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

struct StatefulAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    bool alphaTestEnabled;

    static void invoke(rt::AnyHitInput<Context> input) {}
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
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
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
