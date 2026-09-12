//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly

import slang.raytracing;

struct FirstTag {}
struct SecondTag {}

// Both payload specializations have the same storage. The semantic tag still selects a distinct
// incoming/outgoing Vulkan payload partition.
struct PayloadValue<Tag>
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct FirstMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadValue<FirstTag>;
    typealias Record = void;
}

struct SecondMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadValue<SecondTag>;
    typealias Record = void;
}

struct FirstMiss : rt::IMissShader
{
    typealias Context = FirstMissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 1;
    }
}

struct SecondMiss : rt::IMissShader
{
    typealias Context = SecondMissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 1;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<FirstMiss, SecondMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

// The trace operation is emitted once with an open `Payload` operand. Normal generic
// specialization must substitute its semantic-type metadata at both call sites; a string captured
// while this helper is open would incorrectly give both calls one identity.
void tracePayload<Payload>(inout Payload payload)
{
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, program, payload);
}

[shader("raygeneration")]
void main()
{
    PayloadValue<FirstTag> first = {};
    tracePayload(first);

    PayloadValue<SecondTag> second = {};
    tracePayload(second);
}

// SPIRV-DAG: OpVariable {{.*}} RayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable {{.*}} RayPayloadKHR {{.*}}Location 1
// SPIRV-DAG: OpVariable {{.*}} IncomingRayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable {{.*}} IncomingRayPayloadKHR {{.*}}Location 1
