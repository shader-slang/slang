//TEST_IGNORE_FILE:

module descriptor_intrinsic_lib;

namespace descriptors
{
public interface IProgramShape {}

// This library fixture uses the standard module's intrinsic annotations. Neither the module
// name nor the declaration name needs to match the standard raytracing library.
__magic_type(TraceProgramDescriptorType)
__intrinsic_type(TraceProgramDescriptor)
public struct ProgramHandle<Program : IProgramShape>
{
    private __init(int unavailable) {}
}

public ProgramHandle<Program> identity<Program : IProgramShape>(ProgramHandle<Program> value)
{
    return value;
}
}
