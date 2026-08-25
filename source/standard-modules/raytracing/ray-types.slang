implementing raytracing;

namespace rt
{

public interface IRayMotion {}

public struct NoMotion : IRayMotion {}

// Reuse the native ray representation so the portable D3D/Vulkan trace path needs no conversion.
// Lowercase properties provide the structural API spelling without adding storage.
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

public struct RayTraversalDesc
{
    public RayDesc ray;
    public RAY_FLAG rayFlags;
    public uint instanceMask;
    public uint sbtOffset;
    public uint sbtStride;
    public uint missIndex;
}

public interface IAccelerationStructure {}

// Portable TLAS-to-BLAS / IAS-to-primitive-AS traversal.
public extension RaytracingAccelerationStructure : IAccelerationStructure {}
public typealias AccelerationStructure = RaytracingAccelerationStructure;

// Metal traversal with an explicit maximum acceleration-structure depth.
[require(metal)]
__intrinsic_type(RaytracingAccelerationStructure)
public struct MultiLevelAccelerationStructure<let maxLevelCount : int>
    : IAccelerationStructure
{}

// Built-in primitive data is exposed as a zero-storage property view. Each property maps
// independently to the corresponding target ABI state.
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

public struct CurveData
{
    public property float parameter
    {
        get;
    }
}

public interface IIntersectionPrimitive
{
    associatedtype Attributes;
}

public interface ICustomIntersectionPrimitive : IIntersectionPrimitive {}

public struct TrianglePrimitive : IIntersectionPrimitive
{
    public typealias Attributes = TriangleData;
}

[require(metal)]
public struct CurvePrimitive : IIntersectionPrimitive
{
    public typealias Attributes = CurveData;
}

public struct BoundingBoxPrimitive<CustomAttributes> : ICustomIntersectionPrimitive
{
    public typealias Attributes = CustomAttributes;
}

} // namespace rt
