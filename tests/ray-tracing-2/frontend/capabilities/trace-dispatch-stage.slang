//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry BadAnyHit -stage anyhit -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry BadAnyHit -stage anyhit -target metal

import slang.raytracing;

struct Payload
{
    float value;
}

struct MyTraceContext : rt::ITraceContext
{
    typealias Payload = Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MyTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = uint;
}

struct BadAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
//CHECK: ^^^^^^ dependencies not compatible on stage
//CHECK: ^^^^^^ 'invoke' requires support for stage 'anyhit', but stage is unsupported.
    {
        rt::RayTracer<Layout> tracer;
        tracer.trace({}, scene, descriptor, input.payload);
//CHECK:                                          ^^^^^^^ conflicting capability requirement
//CHECK:                                          ^^^^^^^ 'payload' requires capability
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = BadAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::HitGroupList<TraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> descriptor;
