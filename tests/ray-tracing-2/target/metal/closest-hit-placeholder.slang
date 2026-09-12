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

struct RealClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value = 42;
    }
}

struct PlaceholderGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct RealGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = RealClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<PlaceholderGroup, RealGroup>;
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

// One real closest-hit stage enables a dense Metal VFT for the payload partition. The
// `NoClosestHit` group at function index zero must therefore receive a signature-compatible no-op
// instead of leaving a host-visible hole in that table.
// METAL-COUNT-2: {{\[\[visible\]\]}} void __slang_structural_rt_
// METAL-LABEL: {{\[\[visible\]\]}} void __slang_structural_rt_{{[0-9a-f]+}}(Schema_rayData_0 thread*
// METAL: return;
// METAL: }
// METAL-LABEL: {{\[\[visible\]\]}} void __slang_structural_rt_{{[0-9a-f]+}}(Schema_rayData_0 thread*
// METAL: value_{{[0-9]+}} = 42U;
