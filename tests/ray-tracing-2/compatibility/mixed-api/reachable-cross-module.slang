//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_legacy_helper.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -entry main -stage raygeneration -target hlsl

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

struct StructuralMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<StructuralMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    LegacyPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    legacyTrace(scene, desc.ray, payload);
}

// CHECK: cannot mix legacy and structural ray-tracing APIs in one selected entry point
// CHECK: legacy ray-tracing API is called here
// CHECK: the same selected entry point reaches the structural API
