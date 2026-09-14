//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/open-empty-payload-context.slang -o tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/open-empty-payload-plugin-a.slang -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -o tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/open-empty-payload-plugin-b.slang -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -o tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_b.slang-module
//TEST:SIMPLE(filecheck=HIT_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -DTEST_HIT_ONLY=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=MISS_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -DTEST_MISS_ONLY=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=SHARED_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -DTEST_SHARED=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=DEDUPE_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -DTEST_DEDUPE=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=CALLABLE_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -DTEST_CALLABLE_ONLY=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=GENERIC_IR): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -DTEST_GENERIC=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5 -conformance "ExplicitGenericHitGroup:ExplicitGenericTag" -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -DTEST_MULTILEVEL=1 -entry main -stage raygeneration -target metal -capability metallib_3_1
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -DTEST_SHARED=1 -entry main -stage raygeneration -target spirv-assembly -profile glsl_460 -emit-spirv-directly
//DIAGNOSTIC_TEST:SIMPLE(filecheck=AMBIGUOUS): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_a.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_plugin_b.slang-module -DTEST_AMBIGUOUS=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5
//DIAGNOSTIC_TEST:SIMPLE(filecheck=NOT_FOUND): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_empty_payload_context.slang-module -DTEST_NOT_FOUND=1 -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;
import structural_open_empty_payload_context;
#ifdef TEST_GENERIC
import structural_open_empty_payload_plugin_a;
public typealias ExplicitGenericHitGroup = GenericHitGroup<uint>;
public typealias ExplicitGenericTag = IGenericHitTag;
#endif

#ifdef TEST_HIT_ONLY
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<HitOnlySchema> program;
#elif defined(TEST_MISS_ONLY)
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<MissOnlySchema> program;
#elif defined(TEST_SHARED)
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<SharedSchema> program;
#elif defined(TEST_DEDUPE)
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<DeduplicatedSchema> program;
#elif defined(TEST_AMBIGUOUS)
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<AmbiguousSchema> program;
#elif defined(TEST_NOT_FOUND)
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<NotFoundSchema> program;
#elif defined(TEST_CALLABLE_ONLY)
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<CallableOnlySchema> program;
#elif defined(TEST_GENERIC)
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<GenericSchema> program;
#elif defined(TEST_MULTILEVEL)
rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<MultilevelSchema> program;
#endif

[shader("raygeneration")]
void main()
{
#ifdef TEST_HIT_ONLY
    traceHitOnly(scene, program);
#elif defined(TEST_MISS_ONLY)
    traceMissOnly(scene, program);
#elif defined(TEST_SHARED)
    traceShared(scene, program);
#elif defined(TEST_DEDUPE)
    traceDeduplicated(scene, program);
#elif defined(TEST_AMBIGUOUS)
    rt::RayTracer<AmbiguousSchema> tracer;
    tracer.trace({}, scene, program);
#elif defined(TEST_NOT_FOUND)
    rt::RayTracer<NotFoundSchema> tracer;
    tracer.trace({}, scene, program);
#elif defined(TEST_CALLABLE_ONLY)
    traceCallableOnly(scene, program);
#elif defined(TEST_GENERIC)
    traceGeneric(scene, program);
#elif defined(TEST_MULTILEVEL)
    rt::RayTracer<MultilevelSchema> tracer;
    tracer.trace({}, scene, program);
#endif
}

// Reaching a completed trace from the precompiled context module proves that its deferred
// decoration and both producer-owned packs survived serialization. The link-time resolver removes
// that marker after selecting the linked hit group's payload.
// HIT_IR-NOT: structuralRayTracingDeferredEmptyPayload
// HIT_IR: [structuralRayTracingHitGroupInfo(%UniqueHitGroup
// HIT_IR: structuralRayTracingTrace({{.*}}%HitOnlySchema{{.*}}%EmptyPayload

// MISS_IR-NOT: structuralRayTracingDeferredEmptyPayload
// MISS_IR: [structuralRayTracingMissShaderInfo({{.*}}%UniqueMiss
// MISS_IR: structuralRayTracingTrace({{.*}}%MissOnlySchema{{.*}}%EmptyPayload

// A linked hit group and miss shader serving one nominal payload resolve to one trace payload.
// SHARED_IR-DAG: [structuralRayTracingHitGroupInfo(%UniqueHitGroup
// SHARED_IR-DAG: [structuralRayTracingMissShaderInfo({{.*}}%UniqueMiss
// SHARED_IR: structuralRayTracingTrace({{.*}}%SharedSchema{{.*}}%EmptyPayload

// The listed and linked entries repeat one semantic payload identity without causing ambiguity.
// DEDUPE_IR: structuralRayTracingHitGroupInfo(%ListedHitGroup
// DEDUPE_IR: structuralRayTracingHitGroupInfo({{.*}}%DeduplicatedHitGroup
// DEDUPE_IR: structuralRayTracingTrace({{.*}}%DeduplicatedSchema{{.*}}%EmptyPayload

// An open callable section cannot affect payload selection, so this trace is eager and never owns
// the deferred marker.
// CALLABLE_IR-NOT: structuralRayTracingDeferredEmptyPayload
// CALLABLE_IR: structuralRayTracingTrace({{.*}}%CallableOnlySchema{{.*}}%EmptyPayload

// A finite generic conformance selected at composition time supplies the empty payload.
// GENERIC_IR: [structuralRayTracingHitGroupInfo(%GenericHitGroup
// GENERIC_IR: structuralRayTracingTrace({{.*}}%GenericSchema{{.*}}%EmptyPayload

// The deferred fallback retains the method value argument and its equality witness, so Metal sees
// the same maximum depth as the original three-argument overload.
// METAL: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::max_levels<3>>

// Payload-location assignment runs after resolution and gives the linked program's synthesized
// outgoing payload its first location.
// SPIRV: OpVariable {{.*}} RayPayloadKHR {{.*}}Location 0

// AMBIGUOUS: linked structural ray-tracing schema has multiple empty payloads
// AMBIGUOUS-DAG: after its open sections are linked
// AMBIGUOUS-DAG: 'EmptyPayload'
// AMBIGUOUS-DAG: 'OtherEmptyPayload'

// NOT_FOUND: linked structural ray-tracing schema has no empty payload
// NOT_FOUND: after its open sections are linked
