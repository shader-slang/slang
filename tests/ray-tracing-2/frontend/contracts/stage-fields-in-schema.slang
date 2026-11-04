// Fields on a stage struct are ordinary object data. Selecting the stage through a schema
// calls its static invoke function and does not supply an object containing those fields.
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

struct StatefulClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    uint state;

    static void invoke(rt::ClosestHitInput<Context> input) {}
}

// Each listed stage selects a static function. Neither stage object is constructed by dispatch.
struct SecondStatefulClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    uint state;

    static void invoke(rt::ClosestHitInput<Context> input) {}
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
