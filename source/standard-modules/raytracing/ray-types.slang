implementing raytracing;

namespace rt
{

public interface IRayMotion {}

public struct NoMotion : IRayMotion {}

public struct RayDesc
{
    public float3 origin;
    public float tMin;
    public float3 direction;
    public float tMax;
}

public struct RayTraversalDesc
{
    public RayDesc ray;
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
public struct MultiLevelAccelerationStructure<let maxLevelCount : int>
    : IAccelerationStructure
{}

// Built-in primitive data becomes a zero-storage property view when stage-input lowering lands.
public struct TriangleData {}

[require(metal)]
public struct CurveData {}

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
