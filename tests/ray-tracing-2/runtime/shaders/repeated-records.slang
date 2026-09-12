//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint recordValue;
    uint dispatchWidth;
}

struct RuntimePayload
{
    uint stage;
    uint recordValue;
}

struct RuntimeRecord
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
    typealias Record = RuntimeRecord;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RuntimePayload;
    typealias Record = RuntimeRecord;
}

struct RuntimeClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 10;
        input.payload.recordValue = input.record.value;
    }
}

struct RuntimeMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 20;
        input.payload.recordValue = input.record.value;
    }
}

struct RuntimeHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RuntimeHitGroup>;
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
    uint physicalRecordIndex = rayIndex & 1;

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex < 2 ? float3(0.25, 0.25, 0.0) : float3(2.0, 2.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    // The host deliberately places the same reflected hit and miss functions in two physical SBT
    // records, with a different RuntimeRecord value in each record. The runtime selector therefore
    // chooses a host-owned record position; it is not a shader-declared structural slot.
    desc.sbtOffset = physicalRecordIndex;
    desc.sbtStride = 0;
    desc.missIndex = physicalRecordIndex;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].recordValue = payload.recordValue;
    results[rayIndex].dispatchWidth = DispatchRaysDimensions().x;
}

// HLSL-DAG: [shader("closesthit")]void RuntimeClosestHit(
// HLSL-DAG: [shader("miss")]void RuntimeMiss(
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
// METAL-DAG: metal::visible_function_table<void(Schema_rayData_0 thread*, uchar device*)> missFunctions_0;
// METAL-DAG: metal::visible_function_table<void(Schema_rayData_0 thread*, uchar device*)> closestHitFunctions_0;
// METAL: _slang_miss_record = {{.*}}missIndex_0 * int(32)
// METAL: _slang_miss_function_index = *((device uint*)_slang_miss_record)
// METAL: [_slang_miss_function_index]({{.*}}, _slang_miss_record + 16)
// METAL: _slang_hit_record_index = {{.*}}sbtStride_0 + {{.*}}sbtOffset_0
// METAL: _slang_hit_record = {{.*}}_slang_hit_record_index * int(32)
// METAL: _slang_hit_function_index = *((device uint*)_slang_hit_record)
// METAL: [_slang_hit_function_index]({{.*}}, _slang_hit_record + 16)
// METAL-DAG: {{\[\[visible\]\]}} void __slang_structural_rt_
// METAL-DAG: {{\[\[visible\]\]}} void __slang_structural_rt_
