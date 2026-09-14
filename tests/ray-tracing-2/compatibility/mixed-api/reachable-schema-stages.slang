//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_schema_legacy_helper.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=TRACE_CHECK, non-exhaustive): -experimental-feature -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_schema_legacy_helper.slang-module -entry traceMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CALL_CHECK, non-exhaustive): -experimental-feature -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_schema_legacy_helper.slang-module -entry callMain -stage raygeneration -target hlsl

import slang.raytracing;
import structural_rt_legacy_helper;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = LegacyPayload;
    typealias Record = void;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = LegacyPayload;
    typealias Primitive = rt::TrianglePrimitive;
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

RaytracingAccelerationStructure legacyScene;

struct LegacyCallingClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        RayDesc ray = {};
        legacyTrace(legacyScene, ray, input.payload);
    }
}

struct StructuralMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct LegacyCallingHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = LegacyCallingClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct LegacyCallingCallable : rt::ICallableShader
{
    typealias Context = CallableContext;

    void invoke(rt::CallableInput<Context> input)
    {
        RayDesc ray = {};
        LegacyPayload payload = {};
        legacyTrace(legacyScene, ray, payload);
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<LegacyCallingHitGroup>;
    typealias MissShaders = rt::MissShaderList<StructuralMiss>;
    typealias CallableShaders = rt::CallableShaderList<LegacyCallingCallable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

// Neither entry point directly calls `legacyTrace`. The runtime selector reaches the legacy call
// through the schema's hit-group or callable metadata, which must therefore participate in the
// linked mixed-API call graph.
[shader("raygeneration")]
void traceMain()
{
    rt::RayTraversalDesc desc = {};
    LegacyPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

[shader("raygeneration")]
void callMain()
{
    CallableData data = {};
    rt::RayTracer<Schema> tracer;
    tracer.callShader<CallableContext>(0, program, data);
}

// TRACE_CHECK: cannot mix legacy and structural ray-tracing APIs in one selected entry point
// TRACE_CHECK: legacy ray-tracing API is called here
// TRACE_CHECK: the same selected entry point reaches the structural API

// CALL_CHECK: cannot mix legacy and structural ray-tracing APIs in one selected entry point
// CALL_CHECK: legacy ray-tracing API is called here
// CALL_CHECK: the same selected entry point reaches the structural API
