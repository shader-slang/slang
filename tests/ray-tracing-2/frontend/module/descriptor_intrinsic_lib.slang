//TEST_IGNORE_FILE:

module descriptor_intrinsic_lib;

namespace descriptors
{
public interface IProgramShape {}

// Intrinsic types use the ordinary module compilation path. Neither the module name nor the
// declaration name needs to match the standard raytracing library.
__intrinsic_type(TraceProgramDescriptor)
public struct ProgramHandle<Program : IProgramShape>
{
    private __init(int unavailable) {}
}
}
