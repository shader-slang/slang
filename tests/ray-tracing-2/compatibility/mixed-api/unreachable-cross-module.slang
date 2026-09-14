//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_legacy_helper.slang-module
//TEST:SIMPLE: -experimental-feature -entry main -stage raygeneration -target hlsl -o unreachable-cross-module.hlsl

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

// Importing a module that contains legacy TraceRay is legal when the selected structural entry
// point does not call that helper. The linked-program check follows executable calls rather than
// treating every imported definition as part of the entry point.
[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    LegacyPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}
