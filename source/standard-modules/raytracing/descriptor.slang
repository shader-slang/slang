implementing raytracing;

namespace rt
{

/// Reserves a ParameterBlock-compatible source shape for future compiler-generated resources.
///
/// The intended Metal lowering will replace these placeholders with schema-specific function
/// tables and record storage after specialization. Portable-target lowering will erase the unused
/// block.
internal struct TraceProgramDescriptorResources<Schema>
    where Schema : ITraceProgramSchema
{
    internal ByteAddressBuffer intersectionFunctions;
    internal ByteAddressBuffer missFunctions;
    internal ByteAddressBuffer closestHitFunctions;
    internal ByteAddressBuffer callableFunctions;
    internal ByteAddressBuffer records;
}

/// Represents the descriptor whose target resources later lowering will derive from `Schema`.
///
/// Later target lowering will specialize its physical representation for the schema. The intended
/// representation has no shader-visible storage on D3D or Vulkan; Metal will use the required
/// function tables and record-data resource.
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{
    internal ParameterBlock<TraceProgramDescriptorResources<Schema>> resources;
}

} // namespace rt
