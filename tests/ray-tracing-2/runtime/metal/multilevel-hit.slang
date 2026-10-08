//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// The transform reads require world_space_data. Both native ID arrays use the actual hit depth,
// rather than the statically declared maximum, because this scene also contains a shorter path.
// METAL: intersector<metal::raytracing::instancing, metal::raytracing::world_space_data, metal::raytracing::max_levels<3>>
// METAL: intersection_result<metal::raytracing::instancing, metal::raytracing::world_space_data, metal::raytracing::max_levels<3>>
// METAL: _slang_result.instance_id
// METAL: _slang_result.instance_count, metal::array_ref<uint>(_slang_result.instance_id, _slang_result.instance_count), metal::array_ref<uint>(_slang_result.user_instance_id, _slang_result.instance_count), _slang_result.object_to_world_transform, _slang_result.world_to_object_transform
// METAL: metal::array_ref<uint> instanceIndex_{{[0-9]+}} {{\[\[}}instance_id]]
// METAL-SAME: metal::array_ref<uint> instanceID_{{[0-9]+}} {{\[\[}}user_instance_id]]

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

uint sumInstanceIDs<C : rt::IHitContext>(rt::ClosestHitInput<C> input)
    where C.TraceContext.AccelerationStructure == rt::MultiLevelAccelerationStructure<3>
{
    uint result = 0;
    for (uint level = 0; level < input.instanceCount; ++level)
        result += input.getInstanceID(level);
    return result;
}

struct RuntimeClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        // Each physical record supplies a distinct increment. This makes the committed-hit record
        // selection observable independently from the AnyHit function selected for the candidate.
        input.payload.stage += input.record.closestHitIncrement;
        uint outerIndex = input.getInstanceIndex(0);
        bool validPath = input.maxInstanceCount == 2;
        if (input.instanceCount == 2)
            validPath = validPath && input.getInstanceIndex(1) == 0
                && sumInstanceIDs(input) == 30 + 2 * outerIndex;
        else
            validPath = validPath && input.instanceCount == 1 && outerIndex == 2
                && sumInstanceIDs(input) == 22;
        // Transforms describe the complete path, including the translated outer instance.
        float3 worldPoint = mul(float4(0.0, 0.0, 0.0, 1.0), input.objectToWorld);
        float3 objectPoint = mul(float4(worldPoint, 1.0), input.worldToObject);
        if (!validPath || abs(worldPoint.x - 2.0 * outerIndex) > 0.001
            || any(abs(objectPoint) > 0.001))
            input.payload.stage = 0;
    }
}

struct RuntimeAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        bool validPath = input.instanceCount == 2 && input.getInstanceIndex(0) == 1
            && input.getInstanceIndex(1) == 0 && input.getInstanceID(0) == 21
            && input.getInstanceID(1) == 11;
        input.payload.stage = validPath ? 112 : 0;
    }
}

// Physical hit record zero deliberately writes a distinct sentinel. The host maps sibling paths
// [0, 0] and [1, 0] to different records even though they share the same innermost instance index.
struct FirstPathAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        bool validPath = input.instanceCount == 2 && input.getInstanceIndex(0) == 0
            && input.getInstanceIndex(1) == 0 && input.getInstanceID(0) == 20
            && input.getInstanceID(1) == 10;
        input.payload.stage = validPath ? 900 : 0;
    }
}

struct FirstPathGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = FirstPathAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

// This instance points directly to the primitive AS, so its actual path is shorter than the
// configured maximum. No accessor may invent a second instance or index past instanceCount.
struct ShortPathAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        bool validPath = input.instanceCount == 1 && input.getInstanceIndex(0) == 2
            && input.getInstanceID(0) == 22;
        input.payload.stage = validPath ? 222 : 0;
    }
}

struct ShortPathGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = ShortPathAnyHit;
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
    typealias HitGroups = rt::HitGroupList<FirstPathGroup, HitGroup, ShortPathGroup>;
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
        desc.ray.Origin = float3(0.25, 0.25, 0.0);
    else if (rayIndex == 1)
        desc.ray.Origin = float3(2.25, 0.25, 0.0);
    else if (rayIndex == 2)
        desc.ray.Origin = float3(4.25, 0.25, 0.0);
    else
        desc.ray.Origin = float3(20.0, 20.0, 0.0);
    desc.ray.Direction = float3(0.0, 0.0, 1.0);
    desc.ray.TMin = 0.001;
    desc.ray.TMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    results[rayIndex] = payload.stage;
}
