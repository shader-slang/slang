//TEST:SIMPLE(filecheck=CHECK): -entry main -stage compute -dump-ir -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_ALIAS): -DTEST_OPTIONAL_ALIAS -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_STRUCT): -DTEST_OPTIONAL_STRUCT -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=STATIC): -DTEST_STATIC -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/descriptor_intrinsic_lib.slang -o tests/ray-tracing-2/frontend/module/descriptor-intrinsic-cache.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/ray-tracing-2/frontend/module/descriptor-intrinsic-cache.slang-module -entry main -stage compute -dump-ir -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_ALIAS): -r tests/ray-tracing-2/frontend/module/descriptor-intrinsic-cache.slang-module -DTEST_OPTIONAL_ALIAS -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_STRUCT): -r tests/ray-tracing-2/frontend/module/descriptor-intrinsic-cache.slang-module -DTEST_OPTIONAL_STRUCT -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=STATIC): -r tests/ray-tracing-2/frontend/module/descriptor-intrinsic-cache.slang-module -DTEST_STATIC -no-codegen
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/ray-tracing-2/frontend/module/intrinsic-descriptor.slang -r tests/ray-tracing-2/frontend/module/descriptor-intrinsic-cache.slang-module -entry main -stage compute -dump-ir -no-codegen

import descriptor_intrinsic_lib;

// A distinct cache filename keeps the first runs on source imports. The later runs check that
// serialization preserves both the descriptor AST type and its ordinary generic declaration.
struct Schema : descriptors::IProgramShape {};
struct OtherSchema : descriptors::IProgramShape {};

typealias Descriptor = descriptors::ProgramHandle<Schema>;

// GLSL compatibility mode normally makes unqualified globals static. Resource classification
// must keep these descriptors host-bound; otherwise the opaque-static diagnostic rejects them.
Descriptor descriptor;
descriptors::ProgramHandle<OtherSchema> otherDescriptor;

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
void main(Descriptor aliasedDescriptor, descriptors::ProgramHandle<OtherSchema> directDescriptor)
{
    // The dedicated AST type must still support inference and substitution through a generic
    // function's ProgramHandle<Program> parameter and result, including after module loading.
    Descriptor copy = descriptors::identity(descriptor);
    descriptors::ProgramHandle<OtherSchema> otherCopy = descriptors::identity(otherDescriptor);
    copy = descriptors::identity(aliasedDescriptor);
    otherCopy = descriptors::identity(directDescriptor);
}
