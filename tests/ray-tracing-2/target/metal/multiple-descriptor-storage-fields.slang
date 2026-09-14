//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target metal -dump-ir-after prepareMetalStructuralRayTracing

import slang.raytracing;

struct PayloadA
{
    uint value;
}

struct PayloadB
{
    float value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContextA : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadA;
    typealias Record = void;
}

struct MissContextB : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadB;
    typealias Record = void;
}

struct MissA : rt::IMissShader
{
    typealias Context = MissContextA;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = 1;
    }
}

struct MissB : rt::IMissShader
{
    typealias Context = MissContextB;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = 2.0;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MissA, MissB>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct FrameParameters
{
    rt::AccelerationStructure scene;
    rt::TraceProgramDescriptor<Schema> firstProgram;
    rt::TraceProgramDescriptor<Schema> secondProgram;
    RWStructuredBuffer<uint> output;
}

ParameterBlock<FrameParameters> frame;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};

    PayloadA payloadA = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, frame.scene, frame.firstProgram, payloadA);

    PayloadB payloadB = {};
    tracer.trace(desc, frame.scene, frame.secondProgram, payloadB);
    frame.output[0] = payloadA.value + uint(payloadB.value);
}

// Both descriptor fields use the same schema-specific nominal type in generated Metal.
// METAL-LABEL: struct FrameParameters_default_0
// METAL-DAG: rt_TraceProgramDescriptor_default_0 firstProgram_0;
// METAL-DAG: rt_TraceProgramDescriptor_default_0 secondProgram_0;

// More importantly, both outer field layouts must point at the same physical descriptor layout.
// Schema has two payloads, so that physical layout contains eight argument-buffer resources rather
// than the five source placeholders. Before the regression fix only the first field was rebuilt;
// the second field's var layout still pointed at the five-resource source descriptor layout.
// IR-DAG: [[FIRST_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%firstProgram{{[0-9]*}}, [[FIRST_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[SECOND_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%secondProgram{{[0-9]*}}, [[SECOND_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[FIRST_VAR_LAYOUT]]{{.*}}= varLayout([[PHYSICAL_DESCRIPTOR_LAYOUT:%[0-9]+]]
// IR-DAG: [[SECOND_VAR_LAYOUT]]{{.*}}= varLayout([[PHYSICAL_DESCRIPTOR_LAYOUT]]
// IR-DAG: [[PHYSICAL_DESCRIPTOR_LAYOUT]]{{.*}}= structTypeLayout({{[^,]+}}, [[RESOURCES_FIELD_LAYOUT:%[0-9]+]])
// IR-DAG: [[RESOURCES_FIELD_LAYOUT]]{{.*}}= structFieldLayout(%resources{{[0-9]*}}, [[RESOURCES_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[RESOURCES_VAR_LAYOUT]]{{.*}}= varLayout([[PARAMETER_GROUP_LAYOUT:%[0-9]+]]
// IR-DAG: [[PARAMETER_GROUP_LAYOUT]]{{.*}}= parameterGroupTypeLayout({{[^,]+}}, [[ELEMENT_VAR_LAYOUT:%[0-9]+]], [[PHYSICAL_RESOURCES_LAYOUT:%[0-9]+]])
// IR-DAG: [[ELEMENT_VAR_LAYOUT]]{{.*}}= varLayout([[PHYSICAL_RESOURCES_LAYOUT]]
// IR-DAG: [[PHYSICAL_RESOURCES_LAYOUT]]{{.*}}= structTypeLayout([[ARGUMENT_BUFFER_SIZE:%[0-9]+]]
// IR-DAG: [[ARGUMENT_BUFFER_SIZE]]{{.*}}= size(22 : Int, 8 : Int)
