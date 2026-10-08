//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry rayGenerationMain -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry rayGenerationMain -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=CUDA): -experimental-feature -entry rayGenerationMain -stage raygeneration -target cuda

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint value;
    uint dispatchWidth;
}

// These payloads intentionally have different nominal types and sizes. They share one trace
// program schema and one native SBT, but each trace must select records from its matching payload
// partition.
struct RadiancePayload
{
    uint stage;
    uint radiance;
    uint bounce;
    uint padding[7];
}

struct ShadowPayload
{
    uint stage;
    uint visibility;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct RadianceHitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RadiancePayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct ShadowHitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = ShadowPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct RadianceMissContext : rt::IPayloadContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RadiancePayload;
    typealias Record = void;
}

struct ShadowMissContext : rt::IPayloadContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = ShadowPayload;
    typealias Record = void;
}

// A namespace makes the source lookup names differ from the target-safe entry-point names. The
// host obtains both through reflection and uses each name for its intended purpose.
namespace Stages
{
    // Metal shares one triangle dispatcher across these differently sized payloads. Write through
    // both candidate adapters so the test checks typed ray_data copyback as well as VFT dispatch.
    struct RadianceAnyHit : rt::IAnyHitShader
    {
        typealias Context = RadianceHitContext;

        void invoke(rt::AnyHitInput<Context> input)
        {
            input.payload.radiance = 61;
            for (uint i = 0; i < 7; ++i)
                input.payload.padding[i] = 3 * (i + 1);
        }
    }

    struct ShadowAnyHit : rt::IAnyHitShader
    {
        typealias Context = ShadowHitContext;

        void invoke(rt::AnyHitInput<Context> input)
        {
            input.payload.visibility = input.attributes.barycentricCoord.x > 0 ? 1 : 0;
        }
    }

    struct RadianceClosestHit : rt::IClosestHitShader
    {
        typealias Context = RadianceHitContext;

        void invoke(rt::ClosestHitInput<Context> input)
        {
            input.payload.stage = 10;
            input.payload.radiance += 40;
            input.payload.bounce = 1;
            for (uint i = 0; i < 7; ++i)
                input.payload.padding[i] += 8 * (i + 1);
        }
    }

    struct ShadowClosestHit : rt::IClosestHitShader
    {
        typealias Context = ShadowHitContext;

        void invoke(rt::ClosestHitInput<Context> input)
        {
            input.payload.stage = 20;
            // Keep the visibility written by the shadow candidate; losing copyback must fail.
        }
    }

    struct RadianceMiss : rt::IMissShader
    {
        typealias Context = RadianceMissContext;

        void invoke(rt::MissInput<Context> input)
        {
            input.payload.stage = 11;
            input.payload.radiance = 202;
            input.payload.bounce = 0;
            for (uint i = 0; i < 7; ++i)
                input.payload.padding[i] += 20 * (i + 1);
        }
    }

    struct ShadowMiss : rt::IMissShader
    {
        typealias Context = ShadowMissContext;

        void invoke(rt::MissInput<Context> input)
        {
            input.payload.stage = 21;
            input.payload.visibility = 0;
        }
    }
}

struct RadianceHitGroup : rt::IHitGroup
{
    typealias Context = RadianceHitContext;
    typealias ClosestHit = Stages.RadianceClosestHit;
    typealias AnyHit = Stages.RadianceAnyHit;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct ShadowHitGroup : rt::IHitGroup
{
    typealias Context = ShadowHitContext;
    typealias ClosestHit = Stages.ShadowClosestHit;
    typealias AnyHit = Stages.ShadowAnyHit;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RadianceHitGroup, ShadowHitGroup>;
    typealias MissShaders = rt::MissShaderList<Stages.RadianceMiss, Stages.ShadowMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
// Avoid the reserved C/CUDA name `main`, so OptiX can use the same source and linked name.
void rayGenerationMain()
{
    uint rayIndex = DispatchRaysIndex().x;
    bool usesShadowPayload = (rayIndex & 1) != 0;
    bool hitsTriangle = rayIndex < 2;

    rt::RayTraversalDesc desc = {};
    desc.ray.Origin = hitsTriangle ? float3(0.25, 0.25, 0.0) : float3(2.0, 2.0, 0.0);
    desc.ray.Direction = float3(0.0, 0.0, 1.0);
    desc.ray.TMin = 0.001;
    desc.ray.TMax = 100.0;
    // Candidate adapters must run even when the host's shared triangle scene is marked opaque.
    desc.rayFlags = RAY_FLAG_FORCE_NON_OPAQUE;
    desc.instanceMask = 0xff;

    // The host puts the shadow records before the radiance records. These runtime selectors bind
    // each payload type to its matching physical hit and miss records; schema function indices do
    // not prescribe this physical order.
    uint physicalRecordIndex = usesShadowPayload ? 0 : 1;
    desc.sbtOffset = physicalRecordIndex;
    desc.sbtStride = 0;
    desc.missIndex = physicalRecordIndex;

    rt::RayTracer<Schema> tracer;
    if (usesShadowPayload)
    {
        ShadowPayload payload = {};
        tracer.trace(desc, scene, program, payload);
        results[rayIndex] = { payload.stage, payload.visibility, DispatchRaysDimensions().x };
    }
    else
    {
        RadiancePayload payload = {};
        for (uint i = 0; i < 7; ++i)
            payload.padding[i] = i + 1;
        tracer.trace(desc, scene, program, payload);
        // Include every payload word in the observable result so the OptiX run checks all payload
        // registers. Ten words is also deliberately between the old prelude's eight- and
        // sixteen-register call shapes, so the pipeline's reflected register count must match the
        // exact argument count passed to optixTrace.
        uint value = payload.radiance + 1000 * payload.bounce;
        for (uint i = 0; i < 7; ++i)
            value += payload.padding[i];
        results[rayIndex] = { payload.stage, value, DispatchRaysDimensions().x };
    }
}

// HLSL-COUNT-2: [shader("miss")]void __slang_structural_rt_
// HLSL-COUNT-2: [shader("closesthit")]void __slang_structural_rt_
// SPIRV-COUNT-2: OpEntryPoint MissKHR {{.*}} "__slang_structural_rt_
// SPIRV-COUNT-2: OpEntryPoint ClosestHitKHR {{.*}} "__slang_structural_rt_
// SPIRV-DAG: OpVariable %_ptr_RayPayloadKHR_ShadowPayload RayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable %_ptr_RayPayloadKHR_RadiancePayload RayPayloadKHR {{.*}}Location 1
// SPIRV-DAG: OpVariable %_ptr_IncomingRayPayloadKHR_ShadowPayload IncomingRayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable %_ptr_IncomingRayPayloadKHR_ShadowPayload IncomingRayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable %_ptr_IncomingRayPayloadKHR_RadiancePayload IncomingRayPayloadKHR {{.*}}Location 1
// SPIRV-DAG: OpVariable %_ptr_IncomingRayPayloadKHR_RadiancePayload IncomingRayPayloadKHR {{.*}}Location 1
// CUDA-COUNT-2: __miss____slang_structural_rt_
// CUDA-COUNT-2: __closesthit____slang_structural_rt_
