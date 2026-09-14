implementing raytracing;

namespace rt
{

/// Reserves a ParameterBlock-compatible source shape for compiler-generated descriptor resources.
///
/// Metal lowering replaces these placeholders with schema-specific function tables and record
/// storage after specialization. Other targets erase the unused block.
internal struct TraceProgramDescriptorResources<Schema>
    where Schema : ITraceProgramSchema
{
    internal ByteAddressBuffer intersectionFunctions;
    internal ByteAddressBuffer missFunctions;
    internal ByteAddressBuffer closestHitFunctions;
    internal ByteAddressBuffer callableFunctions;
    internal ByteAddressBuffer records;
}

/// Holds target resources used to dispatch the stages described by `Schema`.
///
/// The compiler specializes its physical representation for the schema and target. It has no
/// shader-visible storage on D3D or Vulkan; Metal lowers it to the required function tables and
/// record-data resource.
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{
    internal ParameterBlock<TraceProgramDescriptorResources<Schema>> resources;
}

} // namespace rt
