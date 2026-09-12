implementing raytracing;

namespace rt
{

/// Identifies the motion model used by every trace and stage in a program schema.
///
/// Applications select one of the standard motion types below. The interface is sealed because
/// lowering must map each motion model to a known target ABI.
[sealed]
public interface IRayMotion {}

/// Marks a motion model that exposes the ray time through stage-input properties.
///
/// This compiler-owned distinction lets the input API omit `time` for `NoMotion` while sharing the
/// same property implementation between the supported motion-enabled models.
[sealed]
public interface IEnabledRayMotion : IRayMotion {}

/// Selects ordinary traversal without a ray-time parameter.
public struct NoMotion : IRayMotion {}

/// Selects Metal primitive motion without moving acceleration-structure instances.
[require(metallib_2_4)]
public struct PrimitiveMotion : IEnabledRayMotion {}

/// Selects instance motion and passes `RayTraversalDesc.time` to native traversal.
///
/// Vulkan and OptiX carry the time in their motion trace operation. Metal uses it with a
/// motion-enabled intersector.
[require(structural_raytracing_instance_motion)]
public struct InstanceMotion : IEnabledRayMotion {}

/// Selects Metal traversal with both primitive and instance motion enabled.
[require(metallib_2_4)]
public struct PrimitiveAndInstanceMotion : IEnabledRayMotion {}

/// Describes a ray origin, interval, and direction.
///
/// This alias reuses the existing native ray representation so portable trace calls need no data
/// conversion. The extension below adds lower-case structural-API property names without storage.
public typealias RayDesc = RayDesc;

public extension RayDesc
{
    public property float3 origin
    {
        get { return Origin; }
        set { Origin = newValue; }
    }

    public property float tMin
    {
        get { return TMin; }
        set { TMin = newValue; }
    }

    public property float3 direction
    {
        get { return Direction; }
        set { Direction = newValue; }
    }

    public property float tMax
    {
        get { return TMax; }
        set { TMax = newValue; }
    }
}

/// Supplies the per-call traversal controls and native SBT selectors.
///
/// A program schema defines which stage programs are available, while `sbtOffset`, `sbtStride`,
/// and `missIndex` select runtime records from the host-built SBT. `time` is consumed only by a
/// motion-enabled trace context.
public struct RayTraversalDesc
{
    public RayDesc ray;
    public float time;
    public RAY_FLAG rayFlags;
    public uint instanceMask;
    public uint sbtOffset;
    public uint sbtStride;
    public uint missIndex;
}

/// Identifies an acceleration-structure topology understood by structural tracing.
public interface IAccelerationStructure {}

/// Makes the existing native acceleration-structure handle usable with the structural API.
///
/// It represents portable two-level TLAS-to-BLAS traversal (or Metal IAS-to-primitive-AS
/// traversal).
public extension RaytracingAccelerationStructure : IAccelerationStructure {}

/// Names the portable two-level acceleration-structure handle.
public typealias AccelerationStructure = RaytracingAccelerationStructure;

/// Represents Metal traversal with an explicit maximum acceleration-structure depth.
///
/// `maxLevelCount` includes every instance level on the path to the primitive. A value of one
/// represents direct primitive-AS traversal; larger values enable Metal's multi-level traversal.
[require(metal)]
__intrinsic_type(RaytracingAccelerationStructure)
public struct MultiLevelAccelerationStructure<let maxLevelCount : int>
    : IAccelerationStructure
{}

/// Exposes the built-in attributes of a triangle hit as a zero-storage property view.
///
/// Each property maps independently to native stage state; the aggregate is not transported as one
/// ABI parameter.
public struct TriangleData
{
    [require(raytracing_anyhit_closesthit)]
    public property float2 barycentricCoord
    {
        get;
    }

    [require(raytracing_anyhit_closesthit)]
    public property bool frontFacing
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
                return HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE;
            default:
                return false;
            }
        }
    }
}

/// Exposes Metal's built-in curve parameter as a zero-storage property view.
public struct CurveData
{
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
    associatedtype Attributes;
}

/// Marks procedural primitives whose intersection stage reports application-defined attributes.
[sealed]
public interface ICustomIntersectionPrimitive : IIntersectionPrimitive {}

/// Selects hardware triangle intersection and the built-in `TriangleData` view.
public struct TrianglePrimitive : IIntersectionPrimitive
{
    public typealias Attributes = TriangleData;
}

/// Selects Metal's built-in curve intersection and `CurveData` view.
[require(metallib_3_1)]
public struct CurvePrimitive : IIntersectionPrimitive
{
    public typealias Attributes = CurveData;
}

/// Selects procedural AABB intersection with application-defined hit attributes.
///
/// An intersection stage reports `CustomAttributes`; the matching any-hit and closest-hit stages
/// read that same type through their stage input.
public struct BoundingBoxPrimitive<CustomAttributes> : ICustomIntersectionPrimitive
{
    public typealias Attributes = CustomAttributes;
}

} // namespace rt
