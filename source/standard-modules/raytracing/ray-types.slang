implementing raytracing;

namespace rt
{

/// Identifies the motion model used by every trace and stage in a program schema.
///
/// Applications select one of the standard motion types below. The interface is sealed because
/// lowering must map each motion model to a known target ABI.
///
/// @par Constraints
/// Only the standard library can add conforming types. Selecting a motion type also selects its
/// target and shader-stage capability requirements.
[sealed]
public interface IRayMotion {}

/// Marks a motion model that exposes the ray time through stage-input properties.
///
/// This compiler-owned distinction lets the input API omit `time` for `NoMotion` while sharing the
/// same property implementation between the supported motion-enabled models.
///
/// @par Constraints
/// Inherits `IRayMotion` and is sealed. `PrimitiveMotion`, `InstanceMotion`, and
/// `PrimitiveAndInstanceMotion` satisfy this contract; `NoMotion` does not.
[sealed]
public interface IEnabledRayMotion : IRayMotion {}

/// Selects ordinary traversal without a ray-time parameter.
///
/// @par Constraints
/// Conforms to `IRayMotion`, but not `IEnabledRayMotion`, so hit and miss inputs do not expose
/// `time` for this motion model.
public struct NoMotion : IRayMotion {}

/// Selects Metal primitive motion without moving acceleration-structure instances.
///
/// @par Constraints
/// Conforms to `IEnabledRayMotion` and requires Metal library version 2.4 or later.
[require(metallib_2_4)]
public struct PrimitiveMotion : IEnabledRayMotion {}

/// Selects instance motion and passes `RayTraversalDesc.time` to native traversal.
///
/// Vulkan and OptiX carry the time in their motion trace operation. Metal uses it with a
/// motion-enabled intersector. The D3D `TraceRay` operation has no ray-time parameter, so D3D
/// programs use `NoMotion` instead; this capability requirement excludes D3D.
///
/// @par Constraints
/// Conforms to `IEnabledRayMotion` and requires `structural_raytracing_instance_motion`.
[require(structural_raytracing_instance_motion)]
public struct InstanceMotion : IEnabledRayMotion {}

/// Selects Metal traversal with both primitive and instance motion enabled.
///
/// @par Constraints
/// Conforms to `IEnabledRayMotion` and requires Metal library version 2.4 or later.
[require(metallib_2_4)]
public struct PrimitiveAndInstanceMotion : IEnabledRayMotion {}

/// Describes a ray origin, interval, and direction.
///
/// This alias reuses the existing ray representation in the core module so trace calls need no
/// data conversion. Its fields are defined as follows:
///
/// ```slang
/// struct RayDesc
/// {
///     float3 Origin;
///     float TMin;
///     float3 Direction;
///     float TMax;
/// };
/// ```
///
/// @par Origin
/// The `float3` ray origin, expressed in the coordinate space of the acceleration structure passed
/// to the trace call. This alias adds no constraint to the core field.
///
/// @par TMin
/// The `float` lower bound of the ray parameter used for traversal. The caller supplies an interval
/// valid for the selected native traversal operation; this alias adds no range validation.
///
/// @par Direction
/// The `float3` ray direction in the same coordinate space as `Origin`. A point at parameter `t`
/// is `Origin + t * Direction`; this alias does not normalize or constrain the vector.
///
/// @par TMax
/// The `float` upper bound of the ray parameter used for traversal. The caller supplies an interval
/// valid for the selected native traversal operation; this alias adds no range validation.
///
/// @par Constraints
/// Exactly the core `RayDesc` type, with no additional conformance or representation requirement.
public typealias RayDesc = RayDesc;

/// Supplies the per-call traversal controls and native SBT selectors.
///
/// A program schema defines which stage programs are available, while `sbtOffset`, `sbtStride`,
/// and `missIndex` select runtime records from the host-built SBT. `time` is consumed only by a
/// motion-enabled trace context.
public struct RayTraversalDesc
{
    /// Supplies the ray origin, direction, and traversal interval.
    ///
    /// @par Constraints
    /// Uses the core `RayDesc` representation. Coordinates are relative to the acceleration
    /// structure passed to the trace call; this field adds no validation to the native ray data.
    public RayDesc ray;

    /// Supplies the sampling time for a motion-enabled trace call.
    ///
    /// @par Constraints
    /// Consumed when the selected motion model implements `IEnabledRayMotion`; a `NoMotion`
    /// context does not consume it. This `float` field adds no range check, so the caller must use
    /// the time domain expected by the target's acceleration structure.
    public float time;

    /// Combines the core module's `RAY_FLAG` constants with bitwise OR; `RAY_FLAG` is a `uint`.
    ///
    /// `RAY_FLAG_NONE` selects default traversal. The other constants control:
    /// - Opacity: `RAY_FLAG_FORCE_OPAQUE` disables any-hit execution;
    ///   `RAY_FLAG_FORCE_NON_OPAQUE` treats geometry as non-opaque.
    /// - Early exit: `RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` accepts the first hit found.
    /// - Stage execution: `RAY_FLAG_SKIP_CLOSEST_HIT_SHADER` skips the closest-hit stage.
    /// - Face culling: `RAY_FLAG_CULL_BACK_FACING_TRIANGLES` and
    ///   `RAY_FLAG_CULL_FRONT_FACING_TRIANGLES` exclude the selected triangle faces.
    /// - Opacity culling: `RAY_FLAG_CULL_OPAQUE` and `RAY_FLAG_CULL_NON_OPAQUE` exclude the
    ///   selected geometry opacity.
    /// - Geometry culling: `RAY_FLAG_SKIP_TRIANGLES` and `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES`
    ///   exclude the selected geometry kind.
    /// - Opacity micromaps: `RAY_FLAG_FORCE_OMM_2_STATE` treats unknown opaque and unknown
    ///   transparent states as opaque and transparent, respectively, on supporting targets.
    ///
    /// @par Constraints
    /// The caller must select flag combinations supported by the target's traversal operation.
    /// This field has type `RAY_FLAG` and does not itself validate combinations or target support.
    public RAY_FLAG rayFlags;

    /// Selects which instances traversal can consider using their instance masks.
    ///
    /// @par Constraints
    /// Uses a `uint`; the caller must choose mask bits supported by the native traversal API and
    /// consistent with the instance masks stored in the acceleration structure.
    public uint instanceMask;

    /// Supplies the ray's offset when selecting a hit-group record from the shader binding table.
    /// A host can use this as the ray-type column of a logical geometry-by-ray-type table.
    ///
    /// @par Constraints
    /// Uses a runtime `uint`, not a schema-list index. The caller must choose this offset together
    /// with `sbtStride` and the acceleration structure's record offsets to select a valid record
    /// in the host-built shader binding table.
    public uint sbtOffset;

    /// Supplies the stride applied to the geometry contribution when selecting a hit-group record.
    /// Counts records rather than bytes. For two ray-type columns per geometry row, use 2.
    ///
    /// @par Constraints
    /// Uses a runtime `uint`. Its value must agree with the host-built shader binding table's
    /// geometry layout and the native traversal operation's supported selector range.
    public uint sbtStride;

    /// Selects the miss record used when traversal finds no accepted hit.
    ///
    /// @par Constraints
    /// Uses a runtime `uint`, not a type position in the schema's miss-shader list. The caller
    /// must select a valid miss record in the host-built shader binding table.
    public uint missIndex;
}

/// Identifies an acceleration-structure topology understood by structural tracing.
///
/// The interface is sealed because lowering maps each supported topology to a fixed native
/// acceleration-structure representation.
///
/// @par Constraints
/// Only the standard library can add conforming types. A trace context's `AccelerationStructure`
/// associated type must satisfy this contract.
[sealed]
public interface IAccelerationStructure {}

/// Makes the existing native acceleration-structure handle usable with the structural API.
///
/// It represents portable two-level TLAS-to-BLAS traversal (or Metal IAS-to-primitive-AS
/// traversal).
///
/// @par Specialization
/// Extends the core `RaytracingAccelerationStructure` type with `IAccelerationStructure`
/// conformance. There are no generic constraints; this conformance identifies that concrete
/// handle as the portable two-level topology.
public extension RaytracingAccelerationStructure : IAccelerationStructure {}

/// Names the portable two-level acceleration-structure handle.
///
/// @par Constraints
/// This alias is exactly `RaytracingAccelerationStructure`, which conforms to the sealed
/// `IAccelerationStructure` interface. It does not denote the multilevel handle family.
public typealias AccelerationStructure = RaytracingAccelerationStructure;

/// Identifies depth-parameterized handles for the multilevel stage-input extensions.
///
/// Extension lookup cannot infer a depth appearing only in a nested associated-type equality.
/// This internal contract lets those extensions select the whole handle family without requiring
/// shader code to supply a second depth argument when reading an instance path.
///
/// @par Constraints
/// Internal and sealed. `MultiLevelAccelerationStructure<N>` supplies this conformance; the
/// portable `AccelerationStructure` type does not.
[sealed]
internal interface IMultiLevelAccelerationStructure
{
    /// Gives the maximum depth, including the primitive acceleration structure and instance levels.
    ///
    /// @par Constraints
    /// Requires a static `uint` constant. The multilevel handle supplies its `maxLevelCount`
    /// argument; this interface does not impose a separate numeric bound.
    static const uint levelCount;
}

/// Represents Metal traversal with an explicit maximum acceleration-structure depth.
///
/// `maxLevelCount` includes the primitive acceleration structure and every instance level on the
/// path to it. A value of one represents direct primitive-AS traversal; larger values enable
/// Metal's multi-level traversal. A hit path therefore contains at most `maxLevelCount - 1`
/// instances.
///
/// @tparam maxLevelCount Maximum depth, including the final primitive acceleration structure.
/// This integer parameter is a type-level constant, not the runtime number of instances hit.
///
/// @par Constraints
/// Requires Metal and conforms to both `IAccelerationStructure` and the internal
/// `IMultiLevelAccelerationStructure` contract. The parameter is declared as `int` with no
/// language-level range constraint; the caller must select a depth supported by Metal.
[require(metal)]
__intrinsic_type(RaytracingAccelerationStructure)
public struct MultiLevelAccelerationStructure<let maxLevelCount : int>
    : IAccelerationStructure, IMultiLevelAccelerationStructure
{
    /// Gives the maximum acceleration-structure depth, including the primitive level.
    ///
    /// @par Constraints
    /// Implements `IMultiLevelAccelerationStructure.levelCount` as a static `uint` constant equal
    /// to `maxLevelCount`. A hit contains at most `levelCount - 1` instance entries; this constant
    /// does not describe the actual path length of a particular hit.
    public static const uint levelCount = maxLevelCount;
}

/// Exposes the built-in attributes of a triangle hit as a zero-storage property view.
///
/// Each property maps independently to native stage state; the aggregate is not transported as one
/// ABI parameter.
public struct TriangleData
{
    /// Reads the two barycentric coordinates supplied for the triangle hit.
    ///
    /// @par Constraints
    /// Requires `structural_raytracing_anyhit_closesthit`, so access is available only in supported
    /// any-hit and closest-hit stages. The coordinates describe built-in triangle intersection
    /// state, rather than application-defined procedural attributes.
    ///
    /// @todo A following PR must connect this getter to native hit attributes or Metal stage state.
    [require(structural_raytracing_anyhit_closesthit)]
    public property float2 barycentricCoord
    {
        get;
    }

    /// Reports whether the ray hit the triangle's front face according to native face orientation.
    ///
    /// @par Constraints
    /// Requires `structural_raytracing_anyhit_closesthit`, so access is available only in supported
    /// any-hit and closest-hit stages. Reads built-in triangle state for the current hit.
    ///
    /// @todo A following PR must synthesize Metal's candidate or committed-hit facing value.
    [require(structural_raytracing_anyhit_closesthit)]
    public property bool frontFacing
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE;
            case metal:
                // TODO: A following PR must synthesize the candidate's [[front_facing]] input or
                // the committed intersection result's triangle_front_facing value.
                static_assert(
                    false,
                    "Metal structural ray-tracing frontFacing requires compiler synthesis");
                return false; // Unreachable; required by return checking.
            }
        }
    }
}

/// Exposes Metal's built-in curve parameter as a zero-storage property view.
///
/// @par Constraints
/// Requires Metal library version 3.1 or later. `CurvePrimitive.Attributes` selects this view for
/// built-in curve hits.
[require(metallib_3_1)]
public struct CurveData
{
    /// Reads the native curve parameter at the hit point.
    ///
    /// @par Constraints
    /// Uses a `float` and inherits `CurveData`'s Metal library version 3.1 requirement. The getter
    /// has no additional declared stage or numeric-range constraint.
    ///
    /// @todo A following PR must synthesize this getter from Metal's candidate or committed curve
    /// data.
    public property float parameter
    {
        get;
    }
}

/// Associates a known geometry kind with the attributes read by its hit stages.
///
/// The interface is sealed because primitive selection controls native signatures and Metal
/// intersection tags. Applications select one of the standard primitive types below.
[sealed]
public interface IIntersectionPrimitive
{
    /// Names the attributes read by the primitive's any-hit and closest-hit stages.
    ///
    /// @par Constraints
    /// This associated type has no conformance constraint. Built-in primitives select their
    /// zero-storage `TriangleData` or `CurveData` view; procedural primitives select the value
    /// type reported by their intersection shader.
    associatedtype Attributes;
}

/// Marks primitives whose intersection is supplied by the target instead of a user shader.
///
/// The interface is sealed so `NoIntersection` can use it to admit exactly the built-in triangle
/// and curve primitives. A procedural bounding box must instead provide an intersection shader
/// that reports an actual hit.
/// For example, a context with `Primitive = TrianglePrimitive` can select `NoIntersection`;
/// one with `Primitive = BoundingBoxPrimitive<MyAttributes>` fails that generic constraint.
///
/// @par Constraints
/// Inherits `IIntersectionPrimitive`, including its `Attributes` requirement, and is sealed.
[sealed]
public interface IBuiltinIntersectionPrimitive : IIntersectionPrimitive {}

/// Marks procedural primitives whose intersection stage reports application-defined attributes.
///
/// `IIntersectionShader` requires this category: a `BoundingBoxPrimitive<MyAttributes>` context
/// supplies the `MyAttributes` value that its intersection shader reports. The any-hit and
/// closest-hit inputs read that value through `attributes`; built-in primitives expose their
/// built-in attribute views through the same property but cannot provide an intersection shader.
///
/// @par Constraints
/// Inherits `IIntersectionPrimitive`, including its `Attributes` requirement, and is sealed.
/// `BoundingBoxPrimitive<CustomAttributes>` supplies this conformance; built-in primitives do not.
[sealed]
public interface ICustomIntersectionPrimitive : IIntersectionPrimitive {}

/// Selects hardware triangle intersection and the built-in `TriangleData` view.
///
/// @par Constraints
/// Conforms to `IBuiltinIntersectionPrimitive`, so a matching hit group uses `NoIntersection`.
public struct TrianglePrimitive : IBuiltinIntersectionPrimitive
{
    /// Selects the built-in triangle attribute view for matching hit-stage inputs.
    ///
    /// @par Constraints
    /// Implements `IIntersectionPrimitive.Attributes` with exactly `TriangleData`. Access to that
    /// view's properties is subject to their any-hit and closest-hit capability requirements.
    public typealias Attributes = TriangleData;
}

/// Selects Metal's built-in curve intersection and `CurveData` view.
///
/// @par Constraints
/// Conforms to `IBuiltinIntersectionPrimitive`, so a matching hit group uses `NoIntersection`.
/// Requires Metal library version 3.1 or later.
[require(metallib_3_1)]
public struct CurvePrimitive : IBuiltinIntersectionPrimitive
{
    /// Selects the built-in curve attribute view for matching hit-stage inputs.
    ///
    /// @par Constraints
    /// Implements `IIntersectionPrimitive.Attributes` with exactly `CurveData`, which also
    /// requires Metal library version 3.1 or later.
    public typealias Attributes = CurveData;
}

/// Selects procedural AABB intersection with application-defined hit attributes.
///
/// An intersection stage reports `CustomAttributes`; the matching any-hit and closest-hit stages
/// read that same type through their stage input.
///
/// @tparam CustomAttributes Attribute value type passed to `reportHit` and read by the matching
/// any-hit and closest-hit stages. The parameter has no declared conformance constraint.
///
/// @par Constraints
/// Conforms to `ICustomIntersectionPrimitive`. A matching hit group must supply an intersection
/// shader; `NoIntersection` requires a built-in primitive and cannot be selected for this type.
public struct BoundingBoxPrimitive<CustomAttributes> : ICustomIntersectionPrimitive
{
    /// Selects the application-defined attributes reported by the intersection shader.
    ///
    /// @par Constraints
    /// Implements `IIntersectionPrimitive.Attributes` with exactly `CustomAttributes`. The
    /// matching intersection stage must report values of that same type.
    public typealias Attributes = CustomAttributes;
}

} // namespace rt
