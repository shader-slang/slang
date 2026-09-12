implementing raytracing;

namespace rt
{

/// Provides the payload, record, hit data, and ray state available to a closest-hit stage.
///
/// This is a compiler-provided zero-storage view. Its properties map directly to native stage
/// parameters or built-in state and therefore do not add fields to the generated entry point.
[require(structural_raytracing_closesthit)]
public struct ClosestHitInput<Context>
    where Context : IHitContext
{
    public property Context.Payload payload
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
            case cuda:
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

    public property uint3 dispatchRaysIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysIndex();
            default:
                return 0;
            }
        }
    }

    public property uint3 dispatchRaysDimensions
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysDimensions();
            default:
                return 0;
            }
        }
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.TraceContext.Motion : IEnabledRayMotion
{
    public property float time
    {
        get
        {
            __target_switch
            {
            case glsl:
            case spirv:
                return RayCurrentTime();
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

/// Provides the candidate payload, record, hit data, and ray state available to an any-hit stage.
///
/// Control methods on this zero-storage view map to native any-hit operations on portable targets
/// and to the generated Metal candidate-dispatch contract.
[require(structural_raytracing_anyhit)]
public struct AnyHitInput<Context>
    where Context : IHitContext
{
    public property Context.Payload payload
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
            case cuda:
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

    public property uint3 dispatchRaysIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysIndex();
            default:
                return 0;
            }
        }
    }

    public property uint3 dispatchRaysDimensions
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysDimensions();
            default:
                return 0;
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
    where Context.TraceContext.Motion : IEnabledRayMotion
{
    public property float time
    {
        get
        {
            __target_switch
            {
            case glsl:
            case spirv:
                return RayCurrentTime();
            default:
                return 0.0;
            }
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

/// Provides the record and ray state used by a procedural intersection stage.
///
/// `reportHit` submits each candidate and its `Context.Primitive.Attributes` value. On D3D and
/// Vulkan it maps to the native report-intersection operation; Metal lowering preserves the same
/// candidate-filtering behavior in the generated intersection function.
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
            case cuda:
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

    public property uint3 dispatchRaysIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysIndex();
            default:
                return 0;
            }
        }
    }

    public property uint3 dispatchRaysDimensions
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysDimensions();
            default:
                return 0;
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
    where Context.TraceContext.Motion : IEnabledRayMotion
{
    public property float time
    {
        get
        {
            __target_switch
            {
            case glsl:
            case spirv:
                return RayCurrentTime();
            default:
                return 0.0;
            }
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

/// Provides the payload, record, and ray state available to a miss stage.
///
/// This zero-storage view lets one source contract map to native miss parameters on portable
/// targets and to Metal's generated post-traversal dispatch.
[require(structural_raytracing_miss)]
public struct MissInput<Context>
    where Context : IPayloadContext
{
    public property Context.Payload payload
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

    public property uint3 dispatchRaysIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysIndex();
            default:
                return 0;
            }
        }
    }

    public property uint3 dispatchRaysDimensions
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysDimensions();
            default:
                return 0;
            }
        }
    }
}

public extension<Context> MissInput<Context>
    where Context : IPayloadContext
    where Context.TraceContext.Motion : IEnabledRayMotion
{
    public property float time
    {
        get
        {
            __target_switch
            {
            case glsl:
            case spirv:
                return RayCurrentTime();
            default:
                return 0.0;
            }
        }
    }
}

/// Provides callable data, record data, and dispatch coordinates to a callable stage.
///
/// The data property is the native inout callable-data parameter; record data identifies the
/// runtime callable SBT record selected by `callShader`.
[require(structural_raytracing_callable)]
public struct CallableInput<Context>
    where Context : ICallableContext
{
    public property Context.CallableData data
    {
        ref;
    }

    public property Context.Record record
    {
        get;
    }

    public property uint3 dispatchRaysIndex
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysIndex();
            default:
                return 0;
            }
        }
    }

    public property uint3 dispatchRaysDimensions
    {
        get
        {
            __target_switch
            {
            case hlsl:
            case glsl:
            case spirv:
            case cuda:
                return DispatchRaysDimensions();
            default:
                return 0;
            }
        }
    }
}

} // namespace rt
