//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimePayload
{
    uint value;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RuntimePayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

Texture2D<uint> values;

struct RuntimeAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.value += values.Load(int3(0, 0, 0));
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = RuntimeAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
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
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

//CHECK: resource cannot be passed directly to a Metal intersection function
//CHECK: Metal intersection functions accept device or constant buffer parameters, not direct texture or sampler parameters
