implementing raytracing;

namespace rt
{

/// Represents a bindable ray-tracing program described by `Schema`.
///
/// Declare this opaque resource as a shader parameter or parameter-block field and pass it to
/// `RayTracer.trace` or `RayTracer.callShader`. It exposes no fields or public constructor; valid
/// values are supplied through host binding.
///
/// The target adapters planned after this frontend slice give it the following representation:
/// - D3D/HLSL: no shader resource or register. The host binds the native pipeline and SBT.
/// - Vulkan/GLSL/SPIR-V: no descriptor-set binding. The host supplies the native SBT regions.
/// - CUDA/OptiX: no shader resource. The host supplies the native OptiX SBT.
/// - Metal: a generated ParameterBlock-like argument buffer containing function tables and a
///   record-data buffer. These are lowering artifacts, not fields available to Slang source.
///
/// For Metal, let `P` be the number of payload partitions in the finalized schema, and `i` a
/// reflected payload-partition index. The intended block-relative argument-buffer IDs are:
///
///     Resource                                  Metal [[id]]
///     intersection function table for payload i  3 * i
///     miss visible-function table for payload i  3 * i + 1
///     closest-hit table for payload i            3 * i + 2
///     program-wide callable table                3 * P
///     program-wide record-data buffer            3 * P + 1
///
/// Thus a one-payload program uses IDs 0 through 4. Each table occupies one argument-buffer ID;
/// its function slots and the host's SBT record indices are separate from these resource bindings.
/// A standalone descriptor's argument buffer is bound at the entry point's reflected `[[buffer(B)]]`
/// index. `B` follows normal parameter binding allocation and is not a fixed slot such as zero.
/// A descriptor inside another ParameterBlock follows that containing parameter's reflected layout.
/// The host must use reflection for the outer binding, payload partitions, and resource IDs rather
/// than infer them from source declaration order. TODO: implement this target layout and its
/// reflection in the adapter slice; this frontend slice diagnoses descriptors reaching codegen.
/// @category raytracing
__magic_type(TraceProgramDescriptorType)
__intrinsic_type(TraceProgramDescriptor)
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{
    // Suppress the synthesized public constructor without declaring a default constructor.
    // Consider `struct Bindings { TraceProgramDescriptor<MySchema> program; }`, bound through
    // `ParameterBlock<Bindings>`. A private zero-argument constructor would still be selected as
    // the field's default initializer while synthesizing Bindings' constructor, then fail access
    // checking. Requiring an argument leaves the descriptor non-default-initializable, so the
    // containing binding struct can carry a host-supplied handle without trying to construct one.
    private __init(int unavailable) {}
}

} // namespace rt
