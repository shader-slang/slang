implementing raytracing;

namespace rt
{

// These placeholder resources reserve a ParameterBlock-compatible physical layout. The Metal
// structural lowering replaces each buffer type with the corresponding function-table or record
// resource type after the concrete ProgramLayout is known. Other targets erase the unused block.
internal struct TraceProgramDescriptorResources<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{
    internal ByteAddressBuffer intersectionFunctions;
    internal ByteAddressBuffer missFunctions;
    internal ByteAddressBuffer closestHitFunctions;
    internal ByteAddressBuffer callableFunctions;
    internal ByteAddressBuffer records;
}

// The compiler specializes the physical representation for ProgramLayout and the target. It has
// no shader-visible storage on D3D or Vulkan; Metal lowers it to the required function-table and
// record-data resources.
public struct TraceProgramDescriptor<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{
    internal ParameterBlock<TraceProgramDescriptorResources<ProgramLayout>> resources;
}

} // namespace rt
