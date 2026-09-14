//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_partition_legacy_helper.slang-module
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_partition_legacy_helper.slang-module -entry main -stage raygeneration -target hlsl -o unreachable-payload-partition.hlsl

import slang.raytracing;
import structural_rt_legacy_helper;

struct ActivePayload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct ActiveMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ActivePayload;
    typealias Record = void;
}

struct InactiveMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = LegacyPayload;
    typealias Record = void;
}

struct InactiveHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = LegacyPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct ActiveMiss : rt::IMissShader
{
    typealias Context = ActiveMissContext;
    void invoke(rt::MissInput<Context> input) {}
}

RaytracingAccelerationStructure unusedLegacyScene;

// This stage is listed in the same schema, but its payload places it in another dispatch
// partition. A trace carrying ActivePayload cannot invoke it, so its legacy call must not make the
// selected structural trace look like mixed API use.
struct InactiveLegacyCallingMiss : rt::IMissShader
{
    typealias Context = InactiveMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        RayDesc ray = {};
        legacyTrace(unusedLegacyScene, ray, input.payload);
    }
}

// Hit-group metadata has its own payload filter in the linked reachability walk. Keep a legacy
// call behind an inactive hit partition as well as the inactive miss partition above so either
// branch becoming over-conservative makes this compile-only test fail.
struct InactiveLegacyCallingClosestHit : rt::IClosestHitShader
{
    typealias Context = InactiveHitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        RayDesc ray = {};
        legacyTrace(unusedLegacyScene, ray, input.payload);
    }
}

struct InactiveLegacyCallingHitGroup : rt::IHitGroup
{
    typealias Context = InactiveHitContext;
    typealias ClosestHit = InactiveLegacyCallingClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<InactiveLegacyCallingHitGroup>;
    typealias MissShaders = rt::MissShaderList<ActiveMiss, InactiveLegacyCallingMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    ActivePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}
