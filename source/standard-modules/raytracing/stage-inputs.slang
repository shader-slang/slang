implementing raytracing;

namespace rt
{

// These are compiler-provided, zero-storage views. The compiler recognizes the exact accessors
// below, maps storage-free properties such as payload to the target ABI, and retains the ordinary
// source bodies where they already express the D3D/Vulkan implementation.
[require(structural_raytracing_closesthit)]
public struct ClosestHitInput<Context>
    where Context : IHitContext
{
    public property Context.TraceContext.Payload payload
    {
        ref;
    }

    public property Context.Record record
    {
        get;
    }

    public property float minDistance
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayTMin();
            default:
                return 0.0;
            }
        }
    }

    public property float distance
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayTCurrent();
            default:
                return 0.0;
            }
        }
    }

    public property uint hitKind
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return HitKind();
            default:
                return 0;
            }
        }
    }

    public property uint rayFlags
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayFlags();
            default:
                return 0;
            }
        }
    }

    public property uint primitiveIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return PrimitiveIndex();
            default:
                return 0;
            }
        }
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
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayOrigin();
            default:
                return 0.0;
            }
        }
    }

    public property float3 worldSpaceDirection
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayDirection();
            default:
                return 0.0;
            }
        }
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.TraceContext.AccelerationStructure == AccelerationStructure
{
    public property uint instanceIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return InstanceIndex();
            default:
                return 0;
            }
        }
    }

    public property uint instanceID
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return InstanceID();
            default:
                return 0;
            }
        }
    }

    public property RayDesc objectSpaceRay
    {
        get
        {
            RayDesc result = {};
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                result.origin = ObjectRayOrigin();
                result.tMin = RayTMin();
                result.direction = ObjectRayDirection();
                result.tMax = RayTCurrent();
            default:
                break;
            }
            return result;
        }
    }

    public property float4x3 objectToWorld
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return ObjectToWorld4x3();
            default:
                return 0.0;
            }
        }
    }

    public property float4x3 worldToObject
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldToObject4x3();
            default:
                return 0.0;
            }
        }
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

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == CurvePrimitive
{
    public property CurveData curve
    {
        get { return {}; }
    }
}

[require(structural_raytracing_anyhit)]
public struct AnyHitInput<Context>
    where Context : IHitContext
{
    public property Context.TraceContext.Payload payload
    {
        ref;
    }

    public property Context.Record record
    {
        get;
    }

    public property float minDistance
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayTMin();
            default:
                return 0.0;
            }
        }
    }

    public property float distance
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayTCurrent();
            default:
                return 0.0;
            }
        }
    }

    public property uint hitKind
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return HitKind();
            default:
                return 0;
            }
        }
    }

    public property uint rayFlags
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayFlags();
            default:
                return 0;
            }
        }
    }

    public property uint primitiveIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return PrimitiveIndex();
            default:
                return 0;
            }
        }
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
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayOrigin();
            default:
                return 0.0;
            }
        }
    }

    public property float3 worldSpaceDirection
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayDirection();
            default:
                return 0.0;
            }
        }
    }

    public void ignoreHit()
    {
        __target_switch
        {
        case hlsl:
        case glsl:
        case spirv:
        case cuda:
            IgnoreHit();
        default:
            break;
        }
    }

    public void acceptHitAndEndSearch()
    {
        __target_switch
        {
        case hlsl:
        case glsl:
        case spirv:
        case cuda:
            AcceptHitAndEndSearch();
        default:
            break;
        }
    }
}

public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.TraceContext.AccelerationStructure == AccelerationStructure
{
    public property uint instanceIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return InstanceIndex();
            default:
                return 0;
            }
        }
    }

    public property uint instanceID
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return InstanceID();
            default:
                return 0;
            }
        }
    }

    public property RayDesc objectSpaceRay
    {
        get
        {
            RayDesc result = {};
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                result.origin = ObjectRayOrigin();
                result.tMin = RayTMin();
                result.direction = ObjectRayDirection();
                result.tMax = RayTCurrent();
            default:
                break;
            }
            return result;
        }
    }

    public property float4x3 objectToWorld
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return ObjectToWorld4x3();
            default:
                return 0.0;
            }
        }
    }

    public property float4x3 worldToObject
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldToObject4x3();
            default:
                return 0.0;
            }
        }
    }
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

public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == CurvePrimitive
{
    public property CurveData curve
    {
        get { return {}; }
    }
}

[require(structural_raytracing_intersection)]
public struct IntersectionInput<Context>
    where Context : IHitContext
{
    public property Context.Record record
    {
        get;
    }

    public property float minDistance
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayTMin();
            default:
                return 0.0;
            }
        }
    }

    public property uint rayFlags
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayFlags();
            default:
                return 0;
            }
        }
    }

    public property RayDesc objectSpaceRay
    {
        get
        {
            RayDesc result = {};
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                result.origin = ObjectRayOrigin();
                result.tMin = RayTMin();
                result.direction = ObjectRayDirection();
                result.tMax = RayTCurrent();
            default:
                break;
            }
            return result;
        }
    }

    public property uint primitiveIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return PrimitiveIndex();
            default:
                return 0;
            }
        }
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
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayOrigin();
            default:
                return 0.0;
            }
        }
    }

    public property float3 worldSpaceDirection
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayDirection();
            default:
                return 0.0;
            }
        }
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

public extension<Context> IntersectionInput<Context>
    where Context : IHitContext
    where Context.TraceContext.AccelerationStructure == AccelerationStructure
{
    public property uint instanceIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return InstanceIndex();
            default:
                return 0;
            }
        }
    }

    public property uint instanceID
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return InstanceID();
            default:
                return 0;
            }
        }
    }

    public property float4x3 objectToWorld
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return ObjectToWorld4x3();
            default:
                return 0.0;
            }
        }
    }

    public property float4x3 worldToObject
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldToObject4x3();
            default:
                return 0.0;
            }
        }
    }
}

[require(structural_raytracing_miss)]
public struct MissInput<Context>
    where Context : IMissGroupContext
{
    public property Context.TraceContext.Payload payload
    {
        ref;
    }

    public property Context.Record record
    {
        get;
    }

    public property float minDistance
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayTMin();
            default:
                return 0.0;
            }
        }
    }

    public property float distance
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayTCurrent();
            default:
                return 0.0;
            }
        }
    }

    public property uint rayFlags
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return RayFlags();
            default:
                return 0;
            }
        }
    }

    public property float3 worldSpaceOrigin
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayOrigin();
            default:
                return 0.0;
            }
        }
    }

    public property float3 worldSpaceDirection
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return WorldRayDirection();
            default:
                return 0.0;
            }
        }
    }
}

[require(structural_raytracing_callable)]
public struct CallableInput<Context>
    where Context : ICallableGroupContext
{
    public property Context.CallableData data
    {
        ref;
    }

    public property Context.Record record
    {
        get;
    }
}

} // namespace rt
