implementing raytracing;

namespace rt
{

// These placeholder resources reserve a ParameterBlock-compatible physical layout. The Metal
// structural lowering replaces each buffer type with the corresponding function-table or record
// resource type after the concrete Schema is known. Other targets erase the unused block.
internal struct TraceProgramDescriptorResources<Schema>
    where Schema : ITraceProgramSchema
{
    internal ByteAddressBuffer intersectionFunctions;
    internal ByteAddressBuffer missFunctions;
    internal ByteAddressBuffer closestHitFunctions;
    internal ByteAddressBuffer callableFunctions;
    internal ByteAddressBuffer records;
}

// The compiler specializes the physical representation for Schema and the target. It has
// no shader-visible storage on D3D or Vulkan; Metal lowers it to the required function-table and
// record-data resources.
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{
    internal ParameterBlock<TraceProgramDescriptorResources<Schema>> resources;
}

} // namespace rt
