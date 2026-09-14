//TEST:SIMPLE(filecheck=CUDA): -experimental-feature -entry main -stage raygeneration -target cuda
//TEST:SIMPLE(filecheck=PTX): -experimental-feature -entry main -stage raygeneration -target ptx

// An instance-motion trace exposes the native ray time to every reachable stage. This schema uses
// all four stage-input contracts so the OptiX adapters cannot silently replace one of them with the
// target-switch fallback value.

import slang.raytracing;

struct Payload
{
    float value;
}

struct Attributes
{
    float value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::InstanceMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = void;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += input.time;
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<Context> input)
    {
        input.payload.value += input.time;
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<Context> input)
    {
        Attributes attributes = {input.time};
        input.reportHit(1.0, attributes);
    }
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.time;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = ::AnyHit;
    typealias Intersection = ::Intersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.375;
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// CUDA-COUNT-4: optixGetRayTime()
// PTX-COUNT-4: _optix_get_ray_time
