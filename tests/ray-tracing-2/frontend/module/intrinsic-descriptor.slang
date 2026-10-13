//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry main -stage compute -dump-ir -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_ALIAS): -experimental-feature -DTEST_OPTIONAL_ALIAS -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_STRUCT): -experimental-feature -DTEST_OPTIONAL_STRUCT -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=STATIC): -experimental-feature -DTEST_STATIC -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/descriptor_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/descriptor-cache.slang-module
//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -r tests/ray-tracing-2/frontend/module/descriptor-cache.slang-module -entry main -stage compute -dump-ir -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_ALIAS): -experimental-feature -r tests/ray-tracing-2/frontend/module/descriptor-cache.slang-module -DTEST_OPTIONAL_ALIAS -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_STRUCT): -experimental-feature -r tests/ray-tracing-2/frontend/module/descriptor-cache.slang-module -DTEST_OPTIONAL_STRUCT -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=STATIC): -experimental-feature -r tests/ray-tracing-2/frontend/module/descriptor-cache.slang-module -DTEST_STATIC -no-codegen
//TEST:SIMPLE_EX(filecheck=CHECK): -experimental-feature -lang glsl tests/ray-tracing-2/frontend/module/intrinsic-descriptor.slang -r tests/ray-tracing-2/frontend/module/descriptor-cache.slang-module -entry main -stage compute -dump-ir -no-codegen

import slang.raytracing;
import descriptor_library;

// A distinct cache filename keeps the first runs on source imports of the application helper.
// The later runs check that its serialized signature still refers to the standard RT descriptor.
struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
};

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
};

struct OtherSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
};

typealias Descriptor = rt::TraceProgramDescriptor<Schema>;

// GLSL compatibility mode normally makes unqualified globals static. Resource classification
// must keep these descriptors host-bound; otherwise the opaque-static diagnostic rejects them.
Descriptor descriptor;
rt::TraceProgramDescriptor<OtherSchema> otherDescriptor;

// Checked generic substitution must preserve each schema and its conformance witness in IR.
// The two specializations stay distinct even though neither handle has source-visible fields.
// CHECK: [nameHint("descriptor")]
// CHECK-NEXT: [export({{.*}})]
// CHECK-NEXT: let %descriptor : TraceProgramDescriptor(%Schema, %{{[0-9]+}}) = global_param
// CHECK: [nameHint("otherDescriptor")]
// CHECK-NEXT: [export({{.*}})]
// CHECK-NEXT: let %otherDescriptor : TraceProgramDescriptor(%OtherSchema, %{{[0-9]+}}) = global_param

struct Bindings
{
    Descriptor descriptor;
};

#if defined(TEST_OPTIONAL_ALIAS)
Optional<Descriptor> optionalDescriptor;
// OPTIONAL_ALIAS: 'Optional<T>' cannot wrap a resource or opaque type
#elif defined(TEST_OPTIONAL_STRUCT)
Optional<Bindings> optionalBindings;
// OPTIONAL_STRUCT: 'Optional<T>' cannot wrap a resource or opaque type
#elif defined(TEST_STATIC)
static Descriptor staticDescriptor;
// STATIC: global variable cannot have opaque type
// STATIC: do you intend to define a `uniform` parameter instead?
#endif

// Descriptor parameters use host bindings without an explicit uniform modifier, through either
// an alias or a direct specialization.
[numthreads(1, 1, 1)]
void main(Descriptor aliasedDescriptor, rt::TraceProgramDescriptor<OtherSchema> directDescriptor)
{
    // The dedicated AST type must still support inference and substitution through a generic
    // function's TraceProgramDescriptor<Program> parameter and result after module loading.
    Descriptor copy = descriptors::identity(descriptor);
    rt::TraceProgramDescriptor<OtherSchema> otherCopy = descriptors::identity(otherDescriptor);
    copy = descriptors::identity(aliasedDescriptor);
    otherCopy = descriptors::identity(directDescriptor);
}
