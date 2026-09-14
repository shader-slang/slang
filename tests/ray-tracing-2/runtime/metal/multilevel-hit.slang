//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// METAL: intersector<metal::raytracing::instancing, metal::raytracing::max_levels<3>>
// METAL: intersection_result<metal::raytracing::instancing, metal::raytracing::max_levels<3>>
// METAL: _slang_result.instance_id
// METAL: metal::array_ref<uint> instanceIndex_{{[0-9]+}} {{\[\[}}instance_id]]

import slang.raytracing;

struct RuntimePayload
{
    uint stage;
}

struct HitRecord
{
    uint closestHitIncrement;
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
    typealias Record = HitRecord;
}

struct RuntimeClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        // Each physical record supplies a distinct increment. This makes the committed-hit record
        // selection observable independently from the AnyHit function selected for the candidate.
        input.payload.stage += input.record.closestHitIncrement;
    }
}

struct RuntimeAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input) { input.payload.stage = 112; }
}

// Physical hit record zero deliberately writes a distinct sentinel. The host maps sibling paths
// [0, 0] and [1, 0] to different records even though they share the same innermost instance index.
struct FirstPathAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input) { input.payload.stage = 900; }
}

struct FirstPathGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = FirstPathAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = RuntimeAnyHit;
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
    void invoke(rt::MissInput<MissContext> input) { input.payload.stage = 2; }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
    // Reflection assigns function indices in this order. The host's instance-path trie maps the
    // sibling paths [0, 0] and [1, 0] to the corresponding physical records.
    typealias HitGroups = rt::HitGroupList<FirstPathGroup, HitGroup>;
    typealias MissShaders = rt::MissShaderList<RuntimeMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> results;

[shader("raygeneration")] void main()
{
    uint rayIndex = DispatchRaysIndex().x;

    rt::RayTraversalDesc desc = {};
    if (rayIndex == 0)
        desc.ray.origin = float3(0.25, 0.25, 0.0);
    else if (rayIndex == 1)
        desc.ray.origin = float3(2.25, 0.25, 0.0);
    else
        desc.ray.origin = float3(20.0, 20.0, 0.0);
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
