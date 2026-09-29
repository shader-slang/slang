implementing raytracing;

namespace rt
{

/// Represents a bindable ray-tracing program described by `Schema`.
///
/// Declare this opaque resource as a shader parameter or parameter-block field, populate the
/// bindings reported by reflection, and pass it to `RayTracer.trace` or `RayTracer.callShader`.
/// The type exposes no fields or public constructor; valid values are supplied through host
/// binding. On targets with a native SBT, this value has no physical shader binding. Metal lowering
/// materializes the function tables and record-data buffer required by the selected program schema.
/// @category raytracing
__magic_type(TraceProgramDescriptorType)
__intrinsic_type(TraceProgramDescriptor)
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{
    // An explicit constructor suppresses the public member-initializer constructor that an empty
    // struct would otherwise receive. Its required private argument also prevents containing
    // aggregates from trying to call it as a default initializer.
    private __init(int unavailable) {}
}

} // namespace rt
