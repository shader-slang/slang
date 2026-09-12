//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -target hlsl -profile sm_6_5 -entry main -stage raygeneration

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

// One type may satisfy both source contracts, but its two native stages cannot claim the same
// exported symbol. The adapter key therefore includes the stage kind and reports the collision.
struct MultiStage : rt::IClosestHitShader, rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) {}
    void invoke(rt::AnyHitInput<HitContext> input) {}
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = MultiStage;
    typealias AnyHit = MultiStage;
    typealias Intersection = rt::NoIntersection<HitContext>;
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
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    /*CHECK:
           ^ conflicting structural ray-tracing entry-point name
           ^ physical entry-point name 'MultiStage' is requested by distinct structural stages
    */
}
