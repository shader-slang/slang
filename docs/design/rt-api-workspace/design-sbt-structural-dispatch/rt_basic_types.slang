implementing rt_pipeline;

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

// The portable TLAS -> BLAS / IAS -> primitive-AS topology.
public struct AccelerationStructure : IAccelerationStructure {}

// An explicitly level-counted Metal topology. A level count of one means
// primitive-AS traversal and omits the instancing tag. Counts of two or greater
// add instancing and max_levels<maxLevelCount>. The compiler will validate the
// supported range and capability when this type is implemented.
[require(metal)]
public struct MultiLevelAccelerationStructure<let maxLevelCount : int>
    : IAccelerationStructure
{}

// Built-in triangle and curve attributes are zero-storage property views. Each
// property maps independently to the corresponding target built-in.
public struct TriangleData
{
    public property float2 barycentricCoord
    {
        get { return __rtTriangleBarycentricCoord(); }
    }

    public property bool frontFacing
    {
        get { return __rtTriangleFrontFacing(); }
    }
}

[require(metal)]
public struct CurveData
{
    public property float parameter
    {
        get { return __rtCurveParameter(); }
    }
}

// Attributes means the primitive-specific data visible to AnyHit and
// ClosestHit. Triangle and curve use built-in property views; a procedural
// primitive uses the user-defined type reported by its Intersection stage.
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

public struct IntersectionResult<AccelerationStructureType, Motion>
    where AccelerationStructureType : IAccelerationStructure
    where Motion : IRayMotion
{
    public bool isNone;
    public float distance;
    public uint primitiveIndex;
    public uint geometryIndex;
}

public interface ITraceContext
{
    associatedtype Payload;
    associatedtype AccelerationStructure : IAccelerationStructure;
    associatedtype Motion : IRayMotion;
}

public interface IHitContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype Primitive : IIntersectionPrimitive;
    associatedtype Record;
}

public interface IMissGroupContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype Record;
}

public interface ICallableGroupContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype CallableData;
    associatedtype Record;
}

// Stage-input values are zero-storage views. Every property maps to a native
// stage parameter or built-in; the declarations below use POC hooks until the
// compiler-known accessors exist.
public struct ClosestHitInput<Context>
    where Context : IHitContext
{
    public property Context.TraceContext.Payload payload
    {
        ref { return __rtClosestHitPayload<Context>(); }
    }

    public property Context.Record record
    {
        get { return __rtClosestHitRecord<Context>(); }
    }

    public property Context.Primitive.Attributes attributes
    {
        get { return __rtClosestHitAttributes<Context>(); }
    }

    public property float distance
    {
        get { return __rtClosestHitDistance<Context>(); }
    }

    public property uint hitKind
    {
        get { return __rtClosestHitKind<Context>(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return __rtClosestHitWorldSpaceOrigin<Context>(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return __rtClosestHitWorldSpaceDirection<Context>(); }
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == TrianglePrimitive
{
    public property TriangleData triangle
    {
        get { return __rtClosestHitTriangle<Context>(); }
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == CurvePrimitive
{
    public property CurveData curve
    {
        get { return __rtClosestHitCurve<Context>(); }
    }
}

public struct AnyHitInput<Context>
    where Context : IHitContext
{
    public property Context.TraceContext.Payload payload
    {
        ref { return __rtAnyHitPayload<Context>(); }
    }

    public property Context.Record record
    {
        get { return __rtAnyHitRecord<Context>(); }
    }

    public property Context.Primitive.Attributes attributes
    {
        get { return __rtAnyHitAttributes<Context>(); }
    }

    public property float distance
    {
        get { return __rtAnyHitDistance<Context>(); }
    }

    public property uint hitKind
    {
        get { return __rtAnyHitKind<Context>(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return __rtAnyHitWorldSpaceOrigin<Context>(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return __rtAnyHitWorldSpaceDirection<Context>(); }
    }

    public void ignoreHit()
    {
        __rtIgnoreHit<Context>();
    }

    public void acceptHitAndEndSearch()
    {
        __rtAcceptHitAndEndSearch<Context>();
    }
}

public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == TrianglePrimitive
{
    public property TriangleData triangle
    {
        get { return __rtAnyHitTriangle<Context>(); }
    }
}

public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == CurvePrimitive
{
    public property CurveData curve
    {
        get { return __rtAnyHitCurve<Context>(); }
    }
}

public struct MissInput<Context>
    where Context : IMissGroupContext
{
    public property Context.TraceContext.Payload payload
    {
        ref { return __rtMissPayload<Context>(); }
    }

    public property Context.Record record
    {
        get { return __rtMissRecord<Context>(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return __rtMissWorldSpaceOrigin<Context>(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return __rtMissWorldSpaceDirection<Context>(); }
    }
}

public struct IntersectionInput<Context>
    where Context : IHitContext
    where Context.Primitive : ICustomIntersectionPrimitive
{
    public property Context.Record record
    {
        get { return __rtIntersectionRecord<Context>(); }
    }

    public property RayDesc objectSpaceRay
    {
        get { return __rtIntersectionObjectSpaceRay<Context>(); }
    }

    public property uint primitiveIndex
    {
        get { return __rtIntersectionPrimitiveIndex<Context>(); }
    }

    public property uint geometryIndex
    {
        get { return __rtIntersectionGeometryIndex<Context>(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return __rtIntersectionWorldSpaceOrigin<Context>(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return __rtIntersectionWorldSpaceDirection<Context>(); }
    }

    public property bool opaque
    {
        get { return __rtIntersectionOpaque<Context>(); }
    }

    // Submit a procedural candidate to traversal. The result is false when the
    // candidate is out of range or rejected by the hit group's AnyHit stage.
    public bool reportHit(float distance, Context.Primitive.Attributes attributes)
    {
        return __rtReportHit<Context>(distance, 0, attributes);
    }

    public bool reportHit(
        float distance,
        uint hitKind,
        Context.Primitive.Attributes attributes)
    {
        return __rtReportHit<Context>(distance, hitKind, attributes);
    }
}

// Callable data is independent of the ray payload. It is the inout value passed
// by an explicit callable-table invocation.
public struct CallableInput<Context>
    where Context : ICallableGroupContext
{
    public property Context.CallableData data
    {
        ref { return __rtCallableData<Context>(); }
    }

    public property Context.Record record
    {
        get { return __rtCallableRecord<Context>(); }
    }
}

// Compiler-known hook declarations used by the zero-storage views above.
Ref<Context.TraceContext.Payload> __rtClosestHitPayload<Context>()
    where Context : IHitContext;
Context.Record __rtClosestHitRecord<Context>() where Context : IHitContext;
Context.Primitive.Attributes __rtClosestHitAttributes<Context>()
    where Context : IHitContext;
float __rtClosestHitDistance<Context>() where Context : IHitContext;
uint __rtClosestHitKind<Context>() where Context : IHitContext;
float3 __rtClosestHitWorldSpaceOrigin<Context>() where Context : IHitContext;
float3 __rtClosestHitWorldSpaceDirection<Context>() where Context : IHitContext;

Ref<Context.TraceContext.Payload> __rtAnyHitPayload<Context>() where Context : IHitContext;
Context.Record __rtAnyHitRecord<Context>() where Context : IHitContext;
Context.Primitive.Attributes __rtAnyHitAttributes<Context>() where Context : IHitContext;
float __rtAnyHitDistance<Context>() where Context : IHitContext;
uint __rtAnyHitKind<Context>() where Context : IHitContext;
float3 __rtAnyHitWorldSpaceOrigin<Context>() where Context : IHitContext;
float3 __rtAnyHitWorldSpaceDirection<Context>() where Context : IHitContext;
void __rtIgnoreHit<Context>() where Context : IHitContext;
void __rtAcceptHitAndEndSearch<Context>() where Context : IHitContext;

Ref<Context.TraceContext.Payload> __rtMissPayload<Context>()
    where Context : IMissGroupContext;
Context.Record __rtMissRecord<Context>() where Context : IMissGroupContext;
float3 __rtMissWorldSpaceOrigin<Context>() where Context : IMissGroupContext;
float3 __rtMissWorldSpaceDirection<Context>() where Context : IMissGroupContext;

Context.Record __rtIntersectionRecord<Context>() where Context : IHitContext;
RayDesc __rtIntersectionObjectSpaceRay<Context>() where Context : IHitContext;
uint __rtIntersectionPrimitiveIndex<Context>() where Context : IHitContext;
uint __rtIntersectionGeometryIndex<Context>() where Context : IHitContext;
float3 __rtIntersectionWorldSpaceOrigin<Context>() where Context : IHitContext;
float3 __rtIntersectionWorldSpaceDirection<Context>() where Context : IHitContext;
bool __rtIntersectionOpaque<Context>() where Context : IHitContext;
bool __rtReportHit<Context>(
    float distance,
    uint hitKind,
    Context.Primitive.Attributes attributes)
    where Context : IHitContext
    where Context.Primitive : ICustomIntersectionPrimitive;

Ref<Context.CallableData> __rtCallableData<Context>()
    where Context : ICallableGroupContext;
Context.Record __rtCallableRecord<Context>() where Context : ICallableGroupContext;

TriangleData __rtClosestHitTriangle<Context>() where Context : IHitContext;
CurveData __rtClosestHitCurve<Context>() where Context : IHitContext;

TriangleData __rtAnyHitTriangle<Context>() where Context : IHitContext;
CurveData __rtAnyHitCurve<Context>() where Context : IHitContext;

// Reachable uses of these compiler-known hooks request triangle_data or
// curve_data for Metal. Other targets map them to their existing built-ins.
float2 __rtTriangleBarycentricCoord();
bool __rtTriangleFrontFacing();
float __rtCurveParameter();

} // namespace rt
