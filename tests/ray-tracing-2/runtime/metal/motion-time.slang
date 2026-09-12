//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal -profile metallib_2_4

// METAL: acceleration_structure<metal::raytracing::instancing, metal::raytracing::primitive_motion>
// METAL: intersector<metal::raytracing::instancing, metal::raytracing::primitive_motion>
// METAL: desc_0.instanceMask_0, desc_0.time_0);

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint timeMillis;
}

struct RuntimePayload
{
    uint stage;
    uint timeMillis;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::PrimitiveMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RuntimePayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct RuntimeClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 1;
        input.payload.timeMillis = uint(input.time * 1000.0 + 0.5);
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RuntimePayload;
    typealias Record = void;
}

struct RuntimeMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 2;
        input.payload.timeMillis = uint(input.time * 1000.0 + 0.5);
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<RuntimeMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;
    float rayTime = rayIndex == 0 ? 0.0 : (rayIndex == 1 ? 1.0 : 0.5);

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex == 0 ? float3(0.25, 0.25, 0.0)
                                    : (rayIndex == 1 ? float3(2.25, 0.25, 0.0)
                                                     : float3(10.0, 10.0, 0.0));
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.time = rayTime;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].timeMillis = payload.timeMillis;
}
