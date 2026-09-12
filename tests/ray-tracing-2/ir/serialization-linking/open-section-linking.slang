//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/open-section-context.slang -o tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module
//TEST:COMPILE: tests/ray-tracing-2/ir/serialization-linking/open-section-plugin-a.slang -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module -o tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_a.slang-module
//TEST:COMPILE: tests/ray-tracing-2/ir/serialization-linking/open-section-plugin-z.slang -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module -o tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_z.slang-module
//TEST:SIMPLE(filecheck=OPEN_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_z.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_a.slang-module -DUSE_OPEN_SECTIONS=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=GENERIC_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_z.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_a.slang-module -DUSE_OPEN_SECTIONS=1 -DUSE_GENERIC_COMPONENT=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -conformance "ExplicitGenericHitGroup:ExplicitGenericTag" -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=OPEN_HLSL): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_z.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_a.slang-module -DUSE_OPEN_SECTIONS=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=OPEN_METAL): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_z.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_a.slang-module -DUSE_OPEN_SECTIONS=1 -entry main -stage raygeneration -target metal -profile metallib_2_4
//TEST:SIMPLE(filecheck=CLOSED_HLSL): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_z.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_plugin_a.slang-module -DUSE_OPEN_SECTIONS=0 -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;
import structural_open_section_context;
import structural_open_section_plugin_a;

#if USE_GENERIC_COMPONENT
public typealias ExplicitGenericHitGroup = CGenericHitGroup<uint>;
public typealias ExplicitGenericTag = IHitTag;
#endif

struct ZListedClosestHit : rt::IClosestHitShader
{
    typealias Context = ListedHitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += 8;
    }
}

// Each listed entry also conforms to its open tag. The completed union must keep it exactly once.
struct ZListedHitGroup : IHitTag
{
    typealias Context = ListedHitContext;
    typealias ClosestHit = ZListedClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct ZListedMiss : IMissTag
{
    typealias Context = ListedMissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 9;
    }
}

struct ZListedCallable : ICallableTag
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input)
    {
        input.data.value += 10;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = SharedTraceContext;
#if USE_OPEN_SECTIONS
    typealias HitGroups = rt::OpenHitGroups<IHitTag, ZListedHitGroup>;
    typealias MissShaders = rt::OpenMissShaders<IMissTag, ZListedMiss>;
    typealias CallableShaders = rt::OpenCallableShaders<ICallableTag, ZListedCallable>;
#else
    typealias HitGroups = rt::HitGroupList<ZListedHitGroup>;
    typealias MissShaders = rt::MissShaderList<ZListedMiss>;
    typealias CallableShaders = rt::CallableShaderList<ZListedCallable>;
#endif
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

#if USE_OPEN_SECTIONS
    LinkedPayload payload = {};
#else
    ListedPayload payload = {};
#endif
    tracer.trace(desc, scene, program, payload);
}

// Listed entries keep declaration order and carry listed origin.
// OPEN_IR-DAG: [structuralRayTracingHitGroupInfo(%ZListedHitGroup{{.*}}, "ZListedHitGroup", {{.*}}, 0 : Int, {{.*}}, false, {{[0-9]+}} : Int)]
// OPEN_IR-DAG: [structuralRayTracingMissShaderInfo(0 : Int, {{.*}}%ZListedMiss{{.*}}, "ZListedMiss", {{.*}}, false, {{[0-9]+}} : Int)]
// OPEN_IR-DAG: [structuralRayTracingCallableShaderInfo(0 : Int, {{.*}}%ZListedCallable{{.*}}, "ZListedCallable", {{.*}}, false)]

// Linked entries follow qualified source-name order within each payload partition. Hit and miss
// indices restart for LinkedPayload, while callable indices are program-wide.
// OPEN_IR-DAG: [structuralRayTracingHitGroupInfo(%ALinkedHitGroup{{.*}}, "ALinkedHitGroup", {{.*}}, 0 : Int, {{.*}}, true, {{[0-9]+}} : Int)]
// OPEN_IR-DAG: [structuralRayTracingHitGroupInfo(%BDerivedHitGroup{{.*}}, "BDerivedHitGroup", {{.*}}, 1 : Int, {{.*}}, true, {{[0-9]+}} : Int)]
// OPEN_IR-DAG: [structuralRayTracingHitGroupInfo(%ZLinkedHitGroup{{.*}}, "ZLinkedHitGroup", {{.*}}, 2 : Int, {{.*}}, true, {{[0-9]+}} : Int)]
// OPEN_IR-DAG: [structuralRayTracingMissShaderInfo(0 : Int, {{.*}}%ALinkedMiss{{.*}}, "ALinkedMiss", {{.*}}, true, {{[0-9]+}} : Int)]
// OPEN_IR-DAG: [structuralRayTracingMissShaderInfo(1 : Int, {{.*}}%ZLinkedMiss{{.*}}, "ZLinkedMiss", {{.*}}, true, {{[0-9]+}} : Int)]
// OPEN_IR-DAG: [structuralRayTracingCallableShaderInfo(1 : Int, {{.*}}%ALinkedCallable{{.*}}, "ALinkedCallable", {{.*}}, true)]
// OPEN_IR-DAG: [structuralRayTracingCallableShaderInfo(2 : Int, {{.*}}%ZLinkedCallable{{.*}}, "ZLinkedCallable", {{.*}}, true)]
// OPEN_IR-NOT: CGenericHitGroup

// An explicitly composed TypeConformance owns one finite generic specialization. Its linked entry
// sorts between the other A-module entries and the Z-module entry.
// GENERIC_IR-DAG: [structuralRayTracingHitGroupInfo(%CGenericHitGroup{{.*}}, "CGenericHitGroup_{{.*}}", {{.*}}, 2 : Int, {{.*}}, true, {{[0-9]+}} : Int)]
// GENERIC_IR-DAG: [structuralRayTracingHitGroupInfo(%ZLinkedHitGroup{{.*}}, "ZLinkedHitGroup", {{.*}}, 3 : Int, {{.*}}, true, {{[0-9]+}} : Int)]
// GENERIC_IR-DAG: [shader("closesthit")]void CGenericClosestHit_

// OPEN_HLSL-DAG: [shader("raygeneration")]void main()
// OPEN_HLSL-DAG: [shader("closesthit")]void ALinkedClosestHit(
// OPEN_HLSL-DAG: [shader("closesthit")]void BDerivedClosestHit(
// OPEN_HLSL-DAG: [shader("closesthit")]void ZLinkedClosestHit(
// OPEN_HLSL-DAG: [shader("miss")]void ALinkedMiss(
// OPEN_HLSL-DAG: [shader("miss")]void ZLinkedMiss(
// OPEN_HLSL-DAG: [shader("callable")]void ALinkedCallable(
// OPEN_HLSL-DAG: [shader("callable")]void ZLinkedCallable(

// Metal creates the linked payload partition's tables and one visible adapter for each listed or
// linked stage. The IR checks above verify the logical indices encoded in their private names.
// OPEN_METAL-DAG: visible_function_table<void(Schema_payload1_rayData_0 thread*)> missFunctions1_0;
// OPEN_METAL-DAG: visible_function_table<void(Schema_payload1_rayData_0 thread*)> closestHitFunctions1_0;
// OPEN_METAL-DAG: visible_function_table<void(SharedCallableData_0 thread*, uint constant*, uint device*)> callableFunctions_0;
// OPEN_METAL-COUNT-10: {{\[\[visible\]\]}} void

// Closed schemas do not select or retain any linked contributor from the same component modules.
// CLOSED_HLSL: [shader("raygeneration")]void main()
// CLOSED_HLSL-NOT: ALinkedClosestHit
// CLOSED_HLSL-NOT: BDerivedClosestHit
// CLOSED_HLSL-NOT: ZLinkedClosestHit
// CLOSED_HLSL-NOT: ALinkedMiss
// CLOSED_HLSL-NOT: ZLinkedMiss
// CLOSED_HLSL-NOT: ALinkedCallable
// CLOSED_HLSL-NOT: ZLinkedCallable
// CLOSED_HLSL-NOT: CGenericClosestHit
