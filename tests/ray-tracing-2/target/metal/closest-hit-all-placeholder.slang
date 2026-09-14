//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

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

struct FirstGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct SecondGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<FirstGroup, SecondGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    output[0] = payload.value;
}

// Both hit-group function indices point at one physical no-op. The compiler must emit the dense
// closest-hit VFT dispatch even though the schema contains no source closest-hit implementation.
// METAL: _slang_intersector.intersect(
// METAL: {{[A-Za-z0-9_]+}}.instanceMask_{{[0-9]+}});
// METAL: _slang_hit_function_index](&rayData_0)
// METAL-COUNT-1: {{\[\[visible\]\]}} void __slang_structural_rt_
// METAL: return;
// This payload has no candidate logic, so it does not pass an intersection-function table.
// METAL-NOT: {{\[\[}}intersection(
