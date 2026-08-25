//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -target hlsl -profile sm_6_5 -entry QueryClosestHit -stage closesthit
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -target spirv-assembly -emit-spirv-directly -entry QueryClosestHit -stage closesthit

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = ::Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

rt::AccelerationStructure scene;

struct QueryClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        RayDesc ray = {};
        ray.Direction = float3(0.0, 0.0, 1.0);
        ray.TMax = 100.0;

        RayQuery<RAY_FLAG_NONE> query;
        query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
        while (query.Proceed())
        {}
        input.payload.value = uint(query.CommittedStatus());
    }
}

// HLSL-DAG: [shader("closesthit")]void QueryClosestHit(
// HLSL-DAG: RayQuery<0U > query
// HLSL-DAG: .TraceRayInline(
// SPIRV: OpEntryPoint ClosestHitKHR %QueryClosestHit "main"
// SPIRV: OpRayQueryInitializeKHR
// SPIRV: OpRayQueryProceedKHR
