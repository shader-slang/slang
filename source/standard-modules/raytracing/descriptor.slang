implementing raytracing;

namespace rt
{

// The compiler specializes the physical representation for ProgramLayout and the target. It has
// no shader-visible storage on D3D or Vulkan; Metal lowers it to the required function-table and
// record-data resources.
public struct TraceProgramDescriptor<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{}

} // namespace rt
