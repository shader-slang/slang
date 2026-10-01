//TEST:SIMPLE(filecheck=HLSL_CLOSEST): -experimental-feature -entry SeparateClosestHit -stage closesthit -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=HLSL_MISS): -experimental-feature -entry SeparateMiss -stage miss -target hlsl -profile sm_6_5

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
    // This 4112-byte array is larger than a native D3D shader-table record can carry and far larger
    // than a 64-DWORD local root signature. Reading it through a runtime index verifies that D3D
    // exposes Record through a real local CBV with ordinary constant-buffer indexing and packing.
    uint4 values[257];
}

struct ExistingConstants
{
    uint bias;
}

// The explicit global b0 binding and compiler-generated local Record binding must coexist in each
// independently compiled stage library. The runtime test also reads both values so neither binding
// can disappear during optimization.
ConstantBuffer<ExistingConstants> existingConstants : register(b0);

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

struct SeparateClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 10;
        input.payload.recordValue =
            input.record.values[255 + input.dispatchRaysIndex.x].x + existingConstants.bias;
    }
}

struct SeparateMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 20;
        input.payload.recordValue =
            input.record.values[255 + input.dispatchRaysIndex.x].x + existingConstants.bias;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SeparateClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<SeparateMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
void rayGenerationMain()
{
    uint rayIndex = DispatchRaysIndex().x;

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex == 0 ? float3(0.25, 0.25, 0.0) : float3(2.0, 2.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 0;
    desc.missIndex = 0;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].recordValue = payload.recordValue;
    results[rayIndex].dispatchWidth = DispatchRaysDimensions().x;
}

// HLSL_CLOSEST-DAG: cbuffer existingConstants{{.*}} : register(b0)
// HLSL_CLOSEST-DAG: cbuffer record{{.*}} : register(b0, space2)
// HLSL_CLOSEST: [shader("closesthit")]void SeparateClosestHit(
// HLSL_MISS-DAG: cbuffer existingConstants{{.*}} : register(b0)
// HLSL_MISS-DAG: cbuffer record{{.*}} : register(b0, space2)
// HLSL_MISS: [shader("miss")]void SeparateMiss(
