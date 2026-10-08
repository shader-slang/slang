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
/// Metal uses one table for each dispatch role, shared by all payload types in the schema.
/// The intended block-relative argument-buffer IDs are:
///
/// @verbatim
///     Resource                            Metal [[id]]
///     intersection function table         0
///     miss visible-function table         1
///     closest-hit visible-function table  2
///     callable visible-function table     3
///     record-data buffer                  4
/// @endverbatim
///
/// Adding a payload type does not add resources or split the SBT record sections. Generated
/// visible-function adapters use a uniform signature per table, with `thread void*` for concrete
/// payload or callable state. Each adapter recovers its concrete type; source stage inputs remain
/// strongly typed. The shared intersection table uses schema-wide intersection tags and generated
/// primitive dispatchers that select candidate logic from the hit record.
///
/// Consider two geometries with radiance and shadow ray types. The host can arrange hit records
/// as one logical two-dimensional table, even when the ray types use different payload types:
///
/// @verbatim
///     Hit records, instanceContribution = 0, sbtStride = 2
///     +--------------+----------------------+----------------------+
///     | Geometry row | Radiance, offset = 0 | Shadow, offset = 1   |
///     +--------------+----------------------+----------------------+
///     | 0            | hit record 0         | hit record 1         |
///     | 1            | hit record 2         | hit record 3         |
///     +--------------+----------------------+----------------------+
///     Miss records: missIndex 0 -> radiance; missIndex 1 -> shadow.
///     Callable records: selected independently by the callShader record index.
/// @endverbatim
///
/// Generated Metal dispatch selects `instanceContribution + geometryIndex * sbtStride + sbtOffset`.
/// `sbtStride` counts records, not bytes. The reflected section stride converts the index to a
/// byte offset.
/// The ray-type columns are a host convention, not payload-type indices. A selected record holds
/// a function index and application record data. Function indices are schema-wide within each
/// section, so several records can name the same program without duplicating its table entry.
/// The host must ensure the selected hit or miss program accepts the trace's payload type.
///
/// Resource IDs, function indices, and SBT record indices are distinct. In particular, Metal's
/// native intersection-table slot selects a primitive dispatcher, which computes the full hit
/// record index above; that slot is not itself the two-dimensional SBT index.
/// A standalone descriptor's argument buffer is bound at the entry point's reflected
/// `[[buffer(B)]]` index. `B` follows normal parameter binding allocation and is not a fixed slot
/// such as zero.
/// A descriptor inside another ParameterBlock follows that containing parameter's reflected layout.
/// The host must use reflection for the outer binding, resource IDs, function indices, and record
/// layouts. TODO: implement the shared-table target layout, adapters, and their
/// reflection in the adapter slice; this frontend slice diagnoses descriptors reaching codegen.
/// @tparam Schema The trace context and finite shader lists describing the bound program.
///
/// @par Constraints
/// `Schema` must conform to `ITraceProgramSchema`.
/// @category raytracing
__magic_type(TraceProgramDescriptorType)
__intrinsic_type(TraceProgramDescriptor)
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{
    /// Suppresses the synthesized public constructor without declaring a default constructor.
    ///
    /// Consider `struct Bindings { TraceProgramDescriptor<MySchema> program; }`, bound through
    /// `ParameterBlock<Bindings>`. A private zero-argument constructor would still be selected as
    /// the field's default initializer while synthesizing `Bindings`' constructor, then fail access
    /// checking. Requiring an argument leaves the descriptor non-default-initializable, so the
    /// containing binding struct can carry a host-supplied handle without trying to construct one.
    /// @param[in] unavailable An unused argument preventing implicit default construction.
    ///
    /// @par Constraints
    /// Private to this type; callers obtain descriptors through host binding.
    private __init(int unavailable) {}
}

} // namespace rt
