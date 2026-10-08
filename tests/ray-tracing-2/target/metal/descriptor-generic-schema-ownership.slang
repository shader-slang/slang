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
    // every descriptor ultimately binds the same number of physical resources.
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

// The generic helper must be specialized once per schema. All three schemas have five resources,
// so the distinct descriptor names prove identity is not inferred from ABI shape. Passing each
// nominal descriptor through a generic helper preserves its schema-specific resource pointer.
// METAL-DAG: OnePayloadSchema_resources{{.*}} constant* identity_{{[0-9]+}}(OnePayloadSchema_resources{{.*}} constant*
// METAL-DAG: OtherOnePayloadSchema_resources{{.*}} constant* identity_{{[0-9]+}}(OtherOnePayloadSchema_resources{{.*}} constant*
// METAL-DAG: TwoPayloadSchema_resources{{.*}} constant* identity_{{[0-9]+}}(TwoPayloadSchema_resources{{.*}} constant*

// PRE: TraceProgramDescriptor
// PRE: TraceProgramDescriptor
// PRE: TraceProgramDescriptor
// IR-NOT: TraceProgramDescriptor(
// IR-DAG: nameHint("OnePayloadSchema.resources")
// IR-DAG: nameHint("OtherOnePayloadSchema.resources")
// IR-DAG: nameHint("TwoPayloadSchema.resources")
// IR-DAG: [[FIRST_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%program{{[0-9]*}}, [[FIRST_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[SECOND_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%program{{[0-9]*}}, [[SECOND_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[THIRD_FIELD_LAYOUT:%[0-9]+]]{{.*}}= structFieldLayout(%program{{[0-9]*}}, [[THIRD_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[FIRST_VAR_LAYOUT]]{{.*}}= varLayout([[FIRST_DESCRIPTOR_LAYOUT:%[0-9]+]]
// IR-DAG: [[SECOND_VAR_LAYOUT]]{{.*}}= varLayout([[SECOND_DESCRIPTOR_LAYOUT:%[0-9]+]]
// IR-DAG: [[THIRD_VAR_LAYOUT]]{{.*}}= varLayout([[THIRD_DESCRIPTOR_LAYOUT:%[0-9]+]]
// IR-DAG: [[FIRST_DESCRIPTOR_LAYOUT]]{{.*}}= parameterGroupTypeLayout({{[^,]+}}, {{%[0-9]+}}, [[FIRST_RESOURCES_LAYOUT:%[0-9]+]],
// IR-DAG: [[SECOND_DESCRIPTOR_LAYOUT]]{{.*}}= parameterGroupTypeLayout({{[^,]+}}, {{%[0-9]+}}, [[SECOND_RESOURCES_LAYOUT:%[0-9]+]],
// IR-DAG: [[THIRD_DESCRIPTOR_LAYOUT]]{{.*}}= parameterGroupTypeLayout({{[^,]+}}, {{%[0-9]+}}, [[THIRD_RESOURCES_LAYOUT:%[0-9]+]],
// IR-DAG: [[FIRST_RESOURCES_LAYOUT]]{{.*}}= structTypeLayout([[RESOURCE_COUNT:%[0-9]+]]
// IR-DAG: [[SECOND_RESOURCES_LAYOUT]]{{.*}}= structTypeLayout([[RESOURCE_COUNT]]
// IR-DAG: [[THIRD_RESOURCES_LAYOUT]]{{.*}}= structTypeLayout([[RESOURCE_COUNT]]
// IR-DAG: [[RESOURCE_COUNT]]{{.*}}= size(22 : Int, 5 : Int)
// IR-NOT: TraceProgramDescriptor(
