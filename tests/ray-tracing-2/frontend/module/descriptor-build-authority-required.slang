//DIAGNOSTIC_TEST:SIMPLE(filecheck=MAGIC): -DTEST_MAGIC
//DIAGNOSTIC_TEST:SIMPLE(filecheck=INTRINSIC): -DTEST_INTRINSIC
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WRONG_DECL): -compile-slang-raytracing-module -DTEST_WRONG_DECL
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WRONG_USE): -compile-slang-raytracing-module -DTEST_WRONG_USE
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WRONG_MAGIC): -compile-slang-raytracing-module -DTEST_WRONG_MAGIC
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WRONG_SHAPE): -compile-slang-raytracing-module -DTEST_WRONG_SHAPE

module raytracing;

namespace rt
{
public interface ITraceProgramSchema {}

#if defined(TEST_WRONG_SHAPE)
// The descriptor opcode requires one constrained schema parameter because those two substitutions
// are the two operands of its IR type.
__magic_type(TraceProgramDescriptorType)
// WRONG_SHAPE: builtin-only modifier on a non-core-module declaration
// WRONG_SHAPE: '__magic_type' can only be applied to declarations in the core module
__intrinsic_type(TraceProgramDescriptor)
//WRONG_SHAPE: compiler-owned intrinsic operation
//WRONG_SHAPE: intrinsic operation 'TraceProgramDescriptor' is reserved for compiler use
public struct TraceProgramDescriptor {}
#elif defined(TEST_WRONG_USE)
// Type-op permission must not leak to other intrinsic modifier forms.
__intrinsic_op(TraceProgramDescriptor) void fakeDescriptorOperation();
//WRONG_USE: compiler-owned intrinsic operation
//WRONG_USE: intrinsic operation 'TraceProgramDescriptor' is reserved for compiler use
#elif defined(TEST_WRONG_DECL)
// The internal build mode authorizes only the canonical descriptor declaration, not arbitrary
// types in the same source compilation.
__intrinsic_type(TraceProgramDescriptor)
//WRONG_DECL: compiler-owned intrinsic operation
//WRONG_DECL: intrinsic operation 'TraceProgramDescriptor' is reserved for compiler use
public struct NotTraceProgramDescriptor<Schema> {}
#elif defined(TEST_MAGIC)
__magic_type(TraceProgramDescriptorType)
//MAGIC: builtin-only modifier
//MAGIC: '__magic_type' can only be applied to declarations in the core module
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{}
#elif defined(TEST_INTRINSIC)
__intrinsic_type(TraceProgramDescriptor)
//INTRINSIC: compiler-owned intrinsic operation
//INTRINSIC: intrinsic operation 'TraceProgramDescriptor' is reserved for compiler use
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{}
#else
__magic_type(TraceProgramDescriptorType)
__magic_type(SamplerStateType)
// WRONG_MAGIC: builtin-only modifier on a non-core-module declaration
// WRONG_MAGIC: '__magic_type' can only be applied to declarations in the core module
__intrinsic_type(TraceProgramDescriptor)
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{}
#endif
}
