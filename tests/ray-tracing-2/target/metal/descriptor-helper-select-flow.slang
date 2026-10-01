//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=NO_FUNCTYPE): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=PRE): -experimental-feature -entry main -stage raygeneration -target metal -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target metal -dump-ir-after prepareMetalStructuralRayTracing

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

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input) { input.payload.value = 1; }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct Programs
{
    rt::TraceProgramDescriptor<Schema> first;
    rt::TraceProgramDescriptor<Schema> second;
    // Unlike the local tuple below, this tuple participates in parameter binding and therefore
    // gives the descriptor layout rewriter a real tuple type/layout pair to update.
    Tuple<rt::TraceProgramDescriptor<Schema>, rt::TraceProgramDescriptor<Schema>> boundAlternatives;
    // This field is intentionally not selected by the trace. Descriptor physicalization follows
    // the canonical schema-carrying type, so unused storage has the same target layout without a
    // value-use graph walk.
    rt::TraceProgramDescriptor<Schema> unused;
}

rt::AccelerationStructure scene;
ParameterBlock<Programs> programs;
uniform bool useSecond;

[noinline]
rt::TraceProgramDescriptor<Schema> chooseProgram(
    rt::TraceProgramDescriptor<Schema> first,
    rt::TraceProgramDescriptor<Schema> second,
    bool selectSecond)
{
    // The conditional lowers through a block parameter (and can become an IR select after CFG
    // simplification), then crosses the helper return/call-result boundary.
    return selectSecond ? second : first;
}

[noinline]
rt::TraceProgramDescriptor<Schema> applyProgramSelector(
    functype(rt::TraceProgramDescriptor<Schema>, rt::TraceProgramDescriptor<Schema>, bool)
        ->rt::TraceProgramDescriptor<Schema> selector,
    rt::TraceProgramDescriptor<Schema> first,
    rt::TraceProgramDescriptor<Schema> second,
    bool selectSecond)
{
    // The descriptor's schema is part of its compiler-owned IR type, so it remains exact across
    // this higher-order signature. Ordinary higher-order specialization can run at its usual point
    // without being coupled to Metal descriptor lowering.
    return selector(first, second, selectSecond);
}

[noinline]
void copyProgram(
    inout rt::TraceProgramDescriptor<Schema> destination,
    rt::TraceProgramDescriptor<Schema> source) { destination = source; }

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;

    // Passing the local as `inout` forces its address to escape. The physical type must therefore
    // cross the pointer parameter and the local's store/load edges as well as `chooseProgram`'s
    // value result.
    let localAlternatives = makeTuple(programs.first, programs.second);
    rt::TraceProgramDescriptor<Schema> selected = localAlternatives._0;
    copyProgram(
        selected,
        applyProgramSelector(
            chooseProgram,
            programs.boundAlternatives._0,
            programs.boundAlternatives._1,
            useSecond));
    tracer.trace(desc, scene, selected, payload);
}

// Both source fields and the helper's complete value flow use the schema-specific physical type.
// METAL-LABEL: struct Programs_default_0
// METAL-DAG: rt_TraceProgramDescriptor_default_0 first_0;
// METAL-DAG: rt_TraceProgramDescriptor_default_0 second_0;
// METAL-DAG: boundAlternatives_0;
// METAL-DAG: rt_TraceProgramDescriptor_default_0 unused_0;
// METAL: void chooseProgram_0(
// METAL-SAME: const rt_TraceProgramDescriptor_default_0 constant* first_{{[0-9]+}},
// METAL-SAME: const rt_TraceProgramDescriptor_default_0 constant* second_{{[0-9]+}},
// METAL-SAME: rt_TraceProgramDescriptorResources_default_0 constant* thread*
// METAL: void copyProgram_0(
// METAL-SAME: rt_TraceProgramDescriptorResources_default_0 constant* thread*
// METAL-SAME: rt_TraceProgramDescriptorResources_default_0 constant*
// METAL: void applyProgramSelector_0(
// METAL-SAME: const rt_TraceProgramDescriptor_default_0 constant*
// METAL-SAME: const rt_TraceProgramDescriptor_default_0 constant*
// METAL-SAME: rt_TraceProgramDescriptorResources_default_0 constant* thread*
// METAL: applyProgramSelector_0(

// Higher-order specialization must remove the source `functype` everywhere, including portions
// of the emitted module outside the ordered `METAL` checks above.
// NO_FUNCTYPE-NOT: Func<
// NO_FUNCTYPE: void applyProgramSelector_0(
// NO_FUNCTYPE-NOT: Func<

// AST-to-IR lowering must preserve the checked Schema argument even though the ordinary source
// storage does not mention it. Metal preparation consumes the wrapper after replacing all of its
// dependent tuple, pointer, function, and user-struct types.
// PRE: StructuralRayTracingProgramDescriptor
// IR-NOT: StructuralRayTracingProgramDescriptor
// IR-DAG: structFieldLayout(%boundAlternatives{{[0-9]*}}, [[BOUND_TUPLE_VAR_LAYOUT:%[0-9]+]])
// IR-DAG: [[BOUND_TUPLE_VAR_LAYOUT]]{{.*}}= varLayout([[BOUND_TUPLE_LAYOUT:%[0-9]+]]
// IR-DAG: [[BOUND_TUPLE_LAYOUT]]{{.*}}= structTypeLayout(
// IR-DAG: nameHint("rt.TraceProgramDescriptor")
// IR-NOT: StructuralRayTracingProgramDescriptor
