implementing raytracing;

namespace rt
{

// These are compiler-provided, zero-storage views. The compiler recognizes the exact accessors
// below, maps storage-free properties such as payload to the target ABI, and retains the ordinary
// source bodies where they already express the D3D/Vulkan implementation.
[require(closesthit)]
public struct ClosestHitInput<Context>
    where Context : IHitContext
{
    public property Context.TraceContext.Payload payload
    {
        ref;
    }

    public property float distance
    {
        get { return RayTCurrent(); }
    }

    public property uint hitKind
    {
        get { return HitKind(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return WorldRayOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return WorldRayDirection(); }
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.Primitive : ICustomIntersectionPrimitive
{
    public property Context.Primitive.Attributes attributes
    {
        get;
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == TrianglePrimitive
{
    public property TriangleData triangle
    {
        get { return {}; }
    }
}

[require(anyhit)]
public struct AnyHitInput<Context>
    where Context : IHitContext
{
    public property Context.TraceContext.Payload payload
    {
        ref;
    }

    public property float distance
    {
        get { return RayTCurrent(); }
    }

    public property uint hitKind
    {
        get { return HitKind(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return WorldRayOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return WorldRayDirection(); }
    }

    public void ignoreHit() { IgnoreHit(); }

    public void acceptHitAndEndSearch() { AcceptHitAndEndSearch(); }
}

public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.Primitive : ICustomIntersectionPrimitive
{
    public property Context.Primitive.Attributes attributes
    {
        get;
    }
}

public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == TrianglePrimitive
{
    public property TriangleData triangle
    {
        get { return {}; }
    }
}

[require(intersection)]
public struct IntersectionInput<Context>
    where Context : IHitContext
{
    public property RayDesc objectSpaceRay
    {
        get
        {
            RayDesc result;
            result.origin = ObjectRayOrigin();
            result.tMin = RayTMin();
            result.direction = ObjectRayDirection();
            result.tMax = RayTCurrent();
            return result;
        }
    }

    public property uint primitiveIndex
    {
        get { return PrimitiveIndex(); }
    }

    public property uint geometryIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
                return GeometryIndex();
            default:
                return 0;
            }
        }
    }

    public property float3 worldSpaceOrigin
    {
        get { return WorldRayOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return WorldRayDirection(); }
    }

    public bool reportHit(float distance, Context.Primitive.Attributes attributes)
    {
        __target_switch
        {
        case hlsl:
        case glsl:
        case spirv:
            return ReportHit(distance, 0, attributes);
        default:
            return false;
        }
    }

    public bool reportHit(
        float distance,
        uint hitKind,
        Context.Primitive.Attributes attributes)
    {
        __target_switch
        {
        case hlsl:
        case glsl:
        case spirv:
            return ReportHit(distance, hitKind, attributes);
        default:
            return false;
        }
    }
}

[require(miss)]
public struct MissInput<Context>
    where Context : IMissGroupContext
{
    public property Context.TraceContext.Payload payload
    {
        ref;
    }

    public property float3 worldSpaceOrigin
    {
        get { return WorldRayOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return WorldRayDirection(); }
    }
}

[require(callable)]
public struct CallableInput<Context>
    where Context : ICallableGroupContext
{}

} // namespace rt
