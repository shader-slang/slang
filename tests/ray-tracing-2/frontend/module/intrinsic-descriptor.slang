//TEST:SIMPLE(filecheck=CHECK): -entry main -stage compute -dump-ir -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_ALIAS): -DTEST_OPTIONAL_ALIAS -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL_STRUCT): -DTEST_OPTIONAL_STRUCT -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=STATIC): -DTEST_STATIC -no-codegen

import descriptor_intrinsic_lib;

struct Schema : descriptors::IProgramShape {}

typealias Descriptor = descriptors::ProgramHandle<Schema>;

Descriptor descriptor;

// An ordinary imported module can declare an intrinsic descriptor. Its checked generic
// substitution supplies both the schema type and its conformance witness to the IR type.
// CHECK: [nameHint("descriptor")]
// CHECK-NEXT: [export({{.*}})]
// CHECK-NEXT: let %descriptor : TraceProgramDescriptor(%Schema, %{{[0-9]+}}) = global_param

struct Bindings
{
    Descriptor descriptor;
}

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

[numthreads(1, 1, 1)]
void main() {}
