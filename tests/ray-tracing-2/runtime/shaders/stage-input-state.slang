//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint minDistance;
    uint distance;
    uint worldOriginX;
    uint worldDirectionZ;
    uint objectOriginX;
    uint objectDirectionZ;
    uint objectToWorldXX;
    uint worldToObjectXX;
    uint primitiveIndex;
    uint geometryIndex;
    uint instanceIndex;
    uint instanceID;
    uint flagsMatch;
    uint dispatchIndex;
    uint dispatchWidth;
}

typealias RuntimePayload = RuntimeResult;

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias Payload = RuntimePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

uint quantize(float value, float scale)
{
    return uint(value * scale + 0.5);
}

struct RuntimeClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        let objectRay = input.objectSpaceRay;
        input.payload.stage = 1;
        input.payload.minDistance = quantize(input.minDistance, 1000.0);
        input.payload.distance = quantize(input.distance, 1000.0);
        input.payload.worldOriginX = quantize(input.worldSpaceOrigin.x, 100.0);
        input.payload.worldDirectionZ = quantize(input.worldSpaceDirection.z, 100.0);
        input.payload.objectOriginX = quantize(objectRay.origin.x, 100.0);
        input.payload.objectDirectionZ = quantize(objectRay.direction.z, 100.0);
        input.payload.objectToWorldXX = quantize(input.objectToWorld[0][0], 100.0);
        input.payload.worldToObjectXX = quantize(input.worldToObject[0][0], 100.0);
        input.payload.primitiveIndex = input.primitiveIndex;
        input.payload.geometryIndex = input.geometryIndex;
        input.payload.instanceIndex = input.instanceIndex;
        input.payload.instanceID = input.instanceID;
        input.payload.flagsMatch =
            input.rayFlags == RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH ? 1 : 0;
        input.payload.dispatchIndex = input.dispatchRaysIndex.x;
        input.payload.dispatchWidth = input.dispatchRaysDimensions.x;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Record = void;
}

struct RuntimeMiss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 2;
        input.payload.minDistance = quantize(input.minDistance, 1000.0);
        input.payload.distance = quantize(input.distance, 1000.0);
        input.payload.worldOriginX = quantize(input.worldSpaceOrigin.x, 100.0);
        input.payload.worldDirectionZ = quantize(input.worldSpaceDirection.z, 100.0);
        input.payload.flagsMatch =
            input.rayFlags == RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH ? 1 : 0;
        input.payload.dispatchIndex = input.dispatchRaysIndex.x;
        input.payload.dispatchWidth = input.dispatchRaysDimensions.x;
    }
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = RuntimeMiss;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RuntimeTraceContext, HitGroup>;
    typealias MissGroups = rt::MissGroupList<RuntimeTraceContext, MissGroup>;
    typealias CallableGroups = rt::NoCallableGroups<RuntimeTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;
    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex == 0 ? float3(0.5, 0.25, 0.0) : float3(3.0, 3.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    results[rayIndex] = payload;
}

// HLSL-DAG: [shader("closesthit")]void RuntimeClosestHit(
// HLSL-DAG: ObjectRayOrigin()
// HLSL-DAG: ObjectToWorld4x3()
// HLSL-DAG: InstanceID()
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
// SPIRV-DAG: BuiltIn ObjectRayOriginKHR
// SPIRV-DAG: BuiltIn ObjectToWorldKHR
// SPIRV-DAG: BuiltIn InstanceCustomIndexKHR
// METAL-DAG: _slang_result.object_to_world_transform
// METAL-DAG: _slang_result.world_to_object_transform
// METAL-DAG: _slang_result.user_instance_id
