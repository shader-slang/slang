//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5

// A user interface may refine the public intersection-stage contract. Structural lowering must
// use the checked conformance from the concrete stage to IIntersectionShader instead of assuming
// that the hit group's IIntersectionStage witness contains exactly one inheritance step.

import slang.raytracing;

struct Payload
{
    uint value;
}

struct Attributes
{
    float value;
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
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = void;
}

interface IApplicationIntersection : rt::IIntersectionShader
{}

struct ApplicationIntersection : IApplicationIntersection
{
    typealias Context = HitContext;

    void invoke(rt::IntersectionInput<HitContext> input)
    {
        Attributes attributes = {1.0};
        input.reportHit(1.0, attributes);
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = ApplicationIntersection;
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
}

// CHECK: [shader("intersection")]
// CHECK: void ApplicationIntersection()
