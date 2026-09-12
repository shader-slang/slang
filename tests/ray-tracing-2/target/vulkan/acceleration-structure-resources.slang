//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -target spirv -emit-spirv-directly -entry main -stage raygeneration -profile sm_6_6 -capability spvDescriptorHeapEXT
//TEST:REFLECTION(filecheck=REFLECT): -experimental-feature -target spirv -entry main -stage raygeneration -profile sm_6_6 -capability spvDescriptorHeapEXT -no-codegen

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissRecord {}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = MissRecord;
}

struct MissShader : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MissShader>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct PushConstants
{
    DescriptorHandle<rt::AccelerationStructure> heapScene;
}

[[vk::binding(0, 0)]] rt::AccelerationStructure scenes[];
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main(uniform PushConstants pushConstants)
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    rt::RayTracer<Schema> tracer;

    Payload arrayPayload = {};
    tracer.trace(desc, scenes[0], program, arrayPayload);

    Payload heapPayload = {};
    tracer.trace(
        desc,
        rt::AccelerationStructure(pushConstants.heapScene),
        program,
        heapPayload);
    output[0] = arrayPayload.value + heapPayload.value;
}

// SPIRV-DAG: OpCapability RuntimeDescriptorArray
// SPIRV-DAG: OpCapability RayTracingKHR
// SPIRV-DAG: %[[AS:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR
// SPIRV-DAG: %[[AS_ARRAY:[A-Za-z0-9_]+]] = OpTypeRuntimeArray %[[AS]]
// SPIRV-NOT: OpDecorate %[[AS_ARRAY]] ArrayStride
// SPIRV: OpConvertUToAccelerationStructureKHR %[[AS]]
// SPIRV: OpTraceRayKHR
// REFLECT-DAG: "name": "scenes"
// REFLECT-DAG: "baseShape": "accelerationStructure"
// REFLECT-DAG: "name": "program"
// REFLECT-DAG: "name": "output"
