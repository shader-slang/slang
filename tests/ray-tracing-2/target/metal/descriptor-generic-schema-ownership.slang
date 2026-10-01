//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=PRE): -experimental-feature -entry main -stage raygeneration -target metal -dump-ir-after specializeModule
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

    void invoke(rt::MissInput<Context> input) { input.payload.value = 1; }
}

struct MissB : rt::IMissShader
{
    typealias Context = MissContextB;

    void invoke(rt::MissInput<Context> input) { input.payload.value = 2.0; }
}

struct OnePayloadSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MissA>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct TwoPayloadSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MissA, MissB>;
    typealias CallableShaders = rt::NoCallableShaders;
}

// This schema has the same physical resource count as `OnePayloadSchema`, but it is still a
// distinct schema identity. Equal ABI shapes must not cause their descriptor types or generic
// helper specializations to merge.
struct OtherOnePayloadSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MissB>;
    typealias CallableShaders = rt::NoCallableShaders;
}

// Both specializations deliberately reuse the declaration-level `program` field key. Their
// concrete fields still have different schema ownership and therefore require different Metal
// descriptor layouts.
struct ProgramHolder<Schema>
    where Schema : rt::ITraceProgramSchema
{
    rt::TraceProgramDescriptor<Schema> program;
}

ParameterBlock<ProgramHolder<OnePayloadSchema>>
    onePayloadProgram;
ParameterBlock<ProgramHolder<OtherOnePayloadSchema>> otherOnePayloadProgram;
ParameterBlock<ProgramHolder<TwoPayloadSchema>> twoPayloadProgram;
rt::AccelerationStructure scene;

[noinline]
rt::TraceProgramDescriptor<Schema> identity<Schema>(
    rt::TraceProgramDescriptor<Schema> program) where Schema : rt::ITraceProgramSchema
{
    // Ordinary generic specialization must produce one helper component per schema even though
    // the source descriptor's Schema parameter is erased from its physical source fields.
    return program;
}

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};

    PayloadA payloadA = {};
    rt::RayTracer<OnePayloadSchema> onePayloadTracer;
    onePayloadTracer
        .trace(desc, scene, identity<OnePayloadSchema>(onePayloadProgram.program), payloadA);

    PayloadB otherPayload = {};
    rt::RayTracer<OtherOnePayloadSchema> otherOnePayloadTracer;
    otherOnePayloadTracer.trace(
        desc,
        scene,
        identity<OtherOnePayloadSchema>(otherOnePayloadProgram.program),
        otherPayload);

    PayloadB payloadB = {};
    rt::RayTracer<TwoPayloadSchema> twoPayloadTracer;
    twoPayloadTracer
        .trace(desc, scene, identity<TwoPayloadSchema>(twoPayloadProgram.program), payloadB);
}

// The generic helper must be specialized once per schema. `OnePayloadSchema` and
// `OtherOnePayloadSchema` both have five resources, so the three explicit descriptor names prove
// identity is not inferred from ABI shape.
// METAL-DAG: void identity_{{[0-9]+}}(const rt_TraceProgramDescriptor_default_0 constant*
// METAL-DAG: void identity_{{[0-9]+}}(const rt_TraceProgramDescriptor_default_1 constant*
// METAL-DAG: void identity_{{[0-9]+}}(const rt_TraceProgramDescriptor_default_2 constant*

// The three concrete schemas remain explicit operands before Metal maps each wrapper to its
// physical descriptor. No wrapper may survive the target preparation boundary.
// PRE: StructuralRayTracingProgramDescriptor
// PRE: StructuralRayTracingProgramDescriptor
// PRE: StructuralRayTracingProgramDescriptor
// IR-NOT: StructuralRayTracingProgramDescriptor

// The three specialized ProgramHolder structs share one source field key, but the paired concrete
// type/layout walk must rebuild three field layouts. Each one-payload descriptor has five
// argument-buffer resources; adding PayloadB gives the two-payload descriptor eight.
// IR-DAG: [[FIRST_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%program{{[0-9]*}}, [[FIRST_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[SECOND_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%program{{[0-9]*}}, [[SECOND_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[THIRD_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%program{{[0-9]*}}, [[THIRD_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[FIRST_VAR_LAYOUT]]{{.*}}= varLayout([[FIRST_DESCRIPTOR_LAYOUT:%[0-9]+]]
// IR-DAG: [[SECOND_VAR_LAYOUT]]{{.*}}= varLayout([[SECOND_DESCRIPTOR_LAYOUT:%[0-9]+]]
// IR-DAG: [[THIRD_VAR_LAYOUT]]{{.*}}= varLayout([[THIRD_DESCRIPTOR_LAYOUT:%[0-9]+]]
// IR-DAG: [[FIRST_DESCRIPTOR_LAYOUT]]{{.*}}= structTypeLayout({{[^,]+}}, [[FIRST_RESOURCES_FIELD_LAYOUT:%[0-9]+]])
// IR-DAG: [[SECOND_DESCRIPTOR_LAYOUT]]{{.*}}= structTypeLayout({{[^,]+}}, [[SECOND_RESOURCES_FIELD_LAYOUT:%[0-9]+]])
// IR-DAG: [[THIRD_DESCRIPTOR_LAYOUT]]{{.*}}= structTypeLayout({{[^,]+}}, [[THIRD_RESOURCES_FIELD_LAYOUT:%[0-9]+]])
// IR-DAG: [[FIRST_RESOURCES_FIELD_LAYOUT]]{{.*}}= structFieldLayout(%resources{{[0-9]*}}, [[FIRST_RESOURCES_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[SECOND_RESOURCES_FIELD_LAYOUT]]{{.*}}= structFieldLayout(%resources{{[0-9]*}}, [[SECOND_RESOURCES_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[THIRD_RESOURCES_FIELD_LAYOUT]]{{.*}}= structFieldLayout(%resources{{[0-9]*}}, [[THIRD_RESOURCES_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[FIRST_RESOURCES_VAR_LAYOUT]]{{.*}}= varLayout([[FIRST_PARAMETER_GROUP_LAYOUT:%[0-9]+]]
// IR-DAG: [[SECOND_RESOURCES_VAR_LAYOUT]]{{.*}}= varLayout([[SECOND_PARAMETER_GROUP_LAYOUT:%[0-9]+]]
// IR-DAG: [[THIRD_RESOURCES_VAR_LAYOUT]]{{.*}}= varLayout([[THIRD_PARAMETER_GROUP_LAYOUT:%[0-9]+]]
// IR-DAG: [[FIRST_PARAMETER_GROUP_LAYOUT]]{{.*}}= parameterGroupTypeLayout({{[^,]+}}, {{%[0-9]+}}, [[FIRST_PHYSICAL_RESOURCES_LAYOUT:%[0-9]+]])
// IR-DAG: [[SECOND_PARAMETER_GROUP_LAYOUT]]{{.*}}= parameterGroupTypeLayout({{[^,]+}}, {{%[0-9]+}}, [[SECOND_PHYSICAL_RESOURCES_LAYOUT:%[0-9]+]])
// IR-DAG: [[THIRD_PARAMETER_GROUP_LAYOUT]]{{.*}}= parameterGroupTypeLayout({{[^,]+}}, {{%[0-9]+}}, [[THIRD_PHYSICAL_RESOURCES_LAYOUT:%[0-9]+]])
// IR-DAG: [[FIRST_PHYSICAL_RESOURCES_LAYOUT]]{{.*}}= structTypeLayout([[TWO_PAYLOAD_RESOURCE_COUNT:%[0-9]+]]
// IR-DAG: [[SECOND_PHYSICAL_RESOURCES_LAYOUT]]{{.*}}= structTypeLayout([[ONE_PAYLOAD_RESOURCE_COUNT:%[0-9]+]]
// IR-DAG: [[THIRD_PHYSICAL_RESOURCES_LAYOUT]]{{.*}}= structTypeLayout([[ONE_PAYLOAD_RESOURCE_COUNT]]
// IR-DAG: [[TWO_PAYLOAD_RESOURCE_COUNT]]{{.*}}= size(22 : Int, 8 : Int)
// IR-DAG: [[ONE_PAYLOAD_RESOURCE_COUNT]]{{.*}}= size(22 : Int, 5 : Int)
