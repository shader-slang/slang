//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint recordValue;
    uint geometryIndex;
    uint dispatchWidth;
}

struct RuntimePayload
{
    uint stage;
    uint recordValue;
    uint geometryIndex;
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
        input.payload.geometryIndex = input.geometryIndex;
    }
}

struct RuntimeMiss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 20;
        input.payload.recordValue = input.record.value;
        input.payload.geometryIndex = 0xffffffff;
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

    rt::RayTraversalDesc desc = {};
    // Geometry zero occupies x=[0, 1], geometry one occupies x=[2, 3], and the third ray misses
    // both. The host places both geometries in one BLAS so `geometryIndex` contributes 0 or 1 to
    // the native SBT addressing formula.
    desc.ray.origin = rayIndex == 0   ? float3(0.25, 0.25, 0.0)
                      : rayIndex == 1 ? float3(2.25, 0.25, 0.0)
                                      : float3(4.25, 0.25, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;

    // The host gives the only TLAS instance a contribution of one. These nonzero selectors make
    // the selected hit record
    //
    //     1 + geometryIndex * 2 + 1,
    //
    // which is physical record two for geometry zero and record four for geometry one. Distinct
    // data in all five records makes omitting or reordering any term visible in the result.
    desc.sbtOffset = 1;
    desc.sbtStride = 2;
    // Miss records form a separate section, so misses use only this explicit nonzero index.
    desc.missIndex = 1;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex] = {
        payload.stage,
        payload.recordValue,
        payload.geometryIndex,
        DispatchRaysDimensions().x,
    };
}

// HLSL-DAG: [shader("closesthit")]void RuntimeClosestHit(
// HLSL-DAG: [shader("miss")]void RuntimeMiss(
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
// METAL: _slang_miss_record =
// METAL: uint _slang_hit_record_index =
// METAL-SAME: _slang_result.instance_id
// METAL-SAME: _slang_result.geometry_id
