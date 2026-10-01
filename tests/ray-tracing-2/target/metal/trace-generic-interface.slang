//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// A source call graph sees `main -> viaInterface -> ITracer.trace`; only witness specialization
// resolves the interface call to `Tracer.trace`, which contains the structural trace. Metal must
// therefore classify the source ray-generation stage before relying on that unspecialized graph,
// then use the specialized IR operation to select its physical compute-kernel implementation.

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

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.value = 2;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<::Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> output;

interface ITracer
{
    uint trace();
}

struct Tracer : ITracer
{
    uint trace()
    {
        rt::RayTraversalDesc desc = {};
        Payload payload = {};
        rt::RayTracer<Schema> tracer;
        tracer.trace(desc, scene, program, payload);
        return payload.value;
    }
}

uint viaInterface<T : ITracer>(T tracer)
{
    return tracer.trace();
}

[shader("raygeneration")]
void main()
{
    output[0] = viaInterface(Tracer());
}

// METAL: uint Tracer_trace_0(
// METAL: metal::raytracing::intersector<metal::raytracing::instancing> _slang_intersector;
// METAL: uint viaInterface_0(
// METAL: Tracer_trace_0(
// METAL: {{\[\[kernel\]\]}} void main_0(
// METAL: viaInterface_0(
