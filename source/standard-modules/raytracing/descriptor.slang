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
/// Metal preserves the SBT record index `i`; each record replaces the native shader identifier
/// with a function index `f` and retains its application data:
///
/// @verbatim
///     i = instanceContribution + geometryIndex * sbtStride + sbtOffset
///
///     D3D/Vulkan hit record i             Metal record at hitSectionOffset + i * hitStride
///     shader identifier / group handle -> header.functionIndex = f
///     application record data          -> Context.Record bytes following the header
///
///     Candidate hit:
///       intersectionTable[primitiveSlot] -> compute i -> read f -> hit group f's candidate logic
///     Committed hit:
///       compute i -> read f -> closestHitTable[f]
/// @endverbatim
///
/// `sbtStride` counts records; `hitStride` counts bytes. The native intersection-table slot selects
/// a primitive dispatcher, not record `i`. The dispatcher selects group `f`'s intersection and
/// any-hit logic. Both paths supply record `i`'s application data through `input.record`.
/// Several records can share `f`. Miss and callable records likewise select their table entries
/// through a function index stored in the record header.
/// @todo Implement the shared Metal tables and record layout in the target-adapter slice.
/// @tparam Schema The trace context and finite shader lists describing the bound program.
///
/// @par Constraints
/// `Schema` must conform to `ITraceProgramSchema`.
/// @category raytracing
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
