//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/generic-interface-legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_generic_interface_legacy_helper.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;
import structural_rt_generic_interface_legacy_helper;

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

struct LegacyCallingMiss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input)
    {
        LegacyTrace implementation = {};
        invokeLegacyTrace(implementation, input.payload);
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<LegacyCallingMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

// The selected entry point contains only a structural trace. Reaching the legacy `TraceRay`
// requires following the schema's runtime-dispatch edge to `LegacyCallingMiss`, then specializing
// the imported generic helper so its interface-witness call resolves to `LegacyTrace.trace`.
[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    LegacyPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// CHECK: cannot mix legacy and structural ray-tracing APIs in one selected entry point
// CHECK: legacy ray-tracing API is called here
// CHECK: the same selected entry point reaches the structural API
