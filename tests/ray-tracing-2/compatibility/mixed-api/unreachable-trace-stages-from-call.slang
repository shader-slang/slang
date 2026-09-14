//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_call_isolation_legacy_helper.slang-module
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_call_isolation_legacy_helper.slang-module -entry main -stage raygeneration -target hlsl -o unreachable-trace-stages-from-call.hlsl

import slang.raytracing;
import structural_rt_legacy_helper;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = LegacyPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = LegacyPayload;
    typealias Record = void;
}

struct CallableData
{
    uint value;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = void;
}

RaytracingAccelerationStructure unusedLegacyScene;

struct LegacyCallingClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        RayDesc ray = {};
        legacyTrace(unusedLegacyScene, ray, input.payload);
    }
}

struct LegacyCallingHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = LegacyCallingClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct LegacyCallingMiss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input)
    {
        RayDesc ray = {};
        legacyTrace(unusedLegacyScene, ray, input.payload);
    }
}

struct CleanCallable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input) { input.data.value += 1; }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<LegacyCallingHitGroup>;
    typealias MissShaders = rt::MissShaderList<LegacyCallingMiss>;
    typealias CallableShaders = rt::CallableShaderList<CleanCallable>;
}

rt::TraceProgramDescriptor<Schema> program;

// A callable dispatch reaches only the schema's callable table. The hit and miss stages above are
// relevant to a trace carrying LegacyPayload, but they cannot run through this selected entry
// point and therefore must not introduce their hidden legacy TraceRay calls into its call graph.
[shader("raygeneration")]
void main()
{
    CallableData data = {};
    rt::RayTracer<Schema> tracer;
    tracer.callShader<CallableContext>(0, program, data);
}
