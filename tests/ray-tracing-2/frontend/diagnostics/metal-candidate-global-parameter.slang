//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimePayload
{
    uint value;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias Payload = RuntimePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

RWStructuredBuffer<uint> values;

struct RuntimeAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.value += values[0];
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = RuntimeAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RuntimeTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<RuntimeTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<RuntimeTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
}

//CHECK: global parameter is unavailable from a Metal intersection function
//CHECK: structural intersection and any-hit logic cannot reference global shader parameters in the first-version Metal lowering
