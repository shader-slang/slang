//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly

import slang.raytracing;

// An explicit legacy payload location is program state even when this declaration is not reached
// from the selected entry point. Structural allocation must reserve it instead of assigning a
// generated incoming/outgoing pair to the same Vulkan location.
[__vulkanRayPayload(0)]
static uint legacyPayload;

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

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 1;
    }
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
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, program, payload);
}

// The unreachable legacy variable itself may be eliminated. Location 1 on both live sides proves
// that its explicit location 0 was nevertheless reserved while assigning the structural payload.
// SPIRV-DAG: OpVariable {{.*}} RayPayloadKHR {{.*}}Location 1
// SPIRV-DAG: OpVariable {{.*}} IncomingRayPayloadKHR {{.*}}Location 1
