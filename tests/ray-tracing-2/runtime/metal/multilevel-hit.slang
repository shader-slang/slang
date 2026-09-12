//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// METAL: intersector<metal::raytracing::instancing, metal::raytracing::max_levels<3>>
// METAL: intersection_result<metal::raytracing::instancing, metal::raytracing::max_levels<3>>
// METAL: _slang_result.instance_id[_slang_result.instance_count - 1]

import slang.raytracing;

struct RuntimePayload
{
    uint stage;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
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
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<RuntimeMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> results;

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex == 0 ? float3(0.25, 0.25, 0.0) : float3(20.0, 20.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    results[rayIndex] = payload.stage;
}
