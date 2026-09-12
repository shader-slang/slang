//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=CUDA): -experimental-feature -entry main -stage raygeneration -target cuda

// InstanceMotion selects the existing target trace operation that accepts ray time. Metal consumes
// the structural marker directly; Vulkan emits OpTraceRayMotionNV, and OptiX passes the same time
// to its native optixTrace helper.

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::InstanceMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.time = 0.375;
    desc.instanceMask = 0xff;

    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// SPIRV-DAG: OpCapability RayTracingMotionBlurNV
// SPIRV-DAG: OpExtension "SPV_NV_ray_tracing_motion_blur"
// SPIRV: OpTraceRayMotionNV

// CUDA: ->time_0 = 0.375
// CUDA: optixTrace({{.*}}.time_0,
