//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

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

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<OnePayloadSchema> onePayloadProgram;
rt::TraceProgramDescriptor<TwoPayloadSchema> twoPayloadProgram;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};

    PayloadA payloadA = {};
    rt::RayTracer<OnePayloadSchema> onePayloadTracer;
    onePayloadTracer.trace(desc, scene, onePayloadProgram, payloadA);

    PayloadB payloadB = {};
    rt::RayTracer<TwoPayloadSchema> twoPayloadTracer;
    twoPayloadTracer.trace(desc, scene, twoPayloadProgram, payloadB);
}

// Specialization erases the phantom schema generic argument from the source descriptor shape.
// Metal lowering must nevertheless retain one physical descriptor type per schema: the first has
// one payload resource triple, while the second has two.
// METAL-LABEL: struct rt_TraceProgramDescriptorResources_default_0
// METAL: intersectionFunctions_0;
// METAL: missFunctions_0;
// METAL: closestHitFunctions_0;
// METAL: callableFunctions_0;
// METAL: records_0;
// METAL-LABEL: struct rt_TraceProgramDescriptorResources_default_1
// METAL: intersectionFunctions0_0;
// METAL: missFunctions0_0;
// METAL: closestHitFunctions0_0;
// METAL: intersectionFunctions1_0;
// METAL: missFunctions1_0;
// METAL: closestHitFunctions1_0;
// METAL: callableFunctions_1;
// METAL: records_1;

// Each source descriptor is retagged only to its own schema's physical representation.
// METAL-LABEL: struct KernelContext_0
// METAL: rt_TraceProgramDescriptorResources_default_0 constant* onePayloadProgram_resources_0;
// METAL: rt_TraceProgramDescriptorResources_default_1 constant* twoPayloadProgram_resources_0;
