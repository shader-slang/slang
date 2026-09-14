//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/higher-order-legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_higher_order_legacy_helper.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_higher_order_legacy_helper.slang-module -entry main -stage raygeneration -target hlsl

import slang.raytracing;
import structural_rt_higher_order_legacy_helper;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct HigherOrderLegacyCallingMiss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input)
    {
        // Before defunctionalization, invokeFunction calls only its function-typed IR parameter.
        // Specialization must reveal legacyTraceLeaf before the mixed-API reachability check.
        invokeFunction(legacyTraceLeaf);
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<HigherOrderLegacyCallingMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// CHECK: cannot mix legacy and structural ray-tracing APIs in one selected entry point
// CHECK: legacy ray-tracing API is called here
// CHECK: the same selected entry point reaches the structural API
