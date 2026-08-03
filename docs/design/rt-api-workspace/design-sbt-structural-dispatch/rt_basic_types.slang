implementing rt_pipeline;

namespace rt
{

public interface IRayASKind {}
public struct InstanceAS : IRayASKind {}

public interface IRayMotion {}
public struct NoMotion : IRayMotion {}

public enum RayDataTags : uint
{
    None = 0,
    Triangle = 0x1,
    Curve = 0x4,
};

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
}

public struct AccelerationStructure<TKind : IRayASKind, TMotion : IRayMotion>
{
}

public interface IIntersectionPrimitive {}
public struct TrianglePrimitive : IIntersectionPrimitive {}
public struct BoundingBoxPrimitive : IIntersectionPrimitive {}
public struct CurvePrimitive : IIntersectionPrimitive {}

public struct TriangleHitAttributes
{
    public float2 barycentricCoord;
    public bool frontFacing;
}

public struct CurveHitAttributes
{
    public float parameter;
}

public struct IntersectionResult<
    TKind : IRayASKind,
    let dataTags : RayDataTags,
    TMotion : IRayMotion,
    let maxLevels : int>
{
    public bool isNone;
    public float distance;
    public uint primitiveIndex;
    public uint geometryIndex;
}

public interface ITraceContext
{
    associatedtype Payload;
    associatedtype ASKind : IRayASKind;
    associatedtype Motion : IRayMotion;

    static const RayDataTags dataTags;
    static const int maxLevels;
}

public interface IHitContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype Primitive : IIntersectionPrimitive;
}

// Marker interface that lets table declarations express "this hit context
// belongs to this trace context" without relying on associated-type equality
// clauses.
public interface IHitContextFor<TTraceContext : ITraceContext> : IHitContext
{
}


void AnyHit(AnyHit<Tcontext> xx)
{

}

[[intersection(, tag_list)]]

public struct ClosestHitInput<TContext : IHitContext>
{
    public TContext.TraceContext.Payload payloadStorage;
    public IntersectionResult<
        TContext.TraceContext.ASKind,
        TContext.TraceContext.dataTags,
        TContext.TraceContext.Motion,
        TContext.TraceContext.maxLevels> intersectionResult;

    public property TContext.TraceContext.Payload payload
    {
        ref { return payloadStorage; }
    }

    public property float distance
    {
        get { return intersectionResult.distance; }
    }

    public property TriangleHitAttributes triangle
    {
        get { return __rtClosestHitTriangle<TContext>(); }
    }

    public property CurveHitAttributes curve
    {
        get { return __rtClosestHitCurve<TContext>(); }
    }

    public TAttributes getCustomAttributes<TAttributes>()
    {
        return __rtClosestHitCustomAttributes<TContext, TAttributes>();
    }
}

public struct AnyHitInput<TContext : IHitContext>
{
    public TContext.TraceContext.Payload payloadStorage;

    public property TContext.TraceContext.Payload payload
    {
        ref { return payloadStorage; }
    }

    public property TriangleHitAttributes triangle
    {
        get { return __rtAnyHitTriangle<TContext>(); }
    }

    public void ignoreHit()
    {
        __rtIgnoreHit<TContext>();
    }
}

public struct MissInput<TTraceContext : ITraceContext>
{
    public TTraceContext.Payload payloadStorage;

    public property TTraceContext.Payload payload
    {
        ref { return payloadStorage; }
    }
}

public struct IntersectionInput<TContext : IHitContext>
{
    public RayDesc ray;
    public uint primitiveIndex;
    public uint geometryIndex;

    public property bool opaque
    {
        get { return __rtIntersectionOpaque<TContext>(); }
    }
}

public struct NoAttributes
{
}

public struct IntersectionReturn<TContext : IHitContext, TAttributes>
{
    public bool acceptIntersection;
    public bool continueSearch;
    public float distance;
    public TAttributes attributes;
}

// Compiler-known hooks used by the public input structs above.

TriangleHitAttributes __rtClosestHitTriangle<TContext : IHitContext>();
CurveHitAttributes __rtClosestHitCurve<TContext : IHitContext>();
TAttributes __rtClosestHitCustomAttributes<TContext : IHitContext, TAttributes>();

TriangleHitAttributes __rtAnyHitTriangle<TContext : IHitContext>();
void __rtIgnoreHit<TContext : IHitContext>();

bool __rtIntersectionOpaque<TContext : IHitContext>();

} // namespace rt
