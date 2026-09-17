implementing raytracing;

namespace rt
{

// Stage-input structs deliberately declare their public properties separately. Although several
// stages expose the same native value, their complete property sets are different; a common public
// base interface would make it too easy to expose a value in a stage where the target ABI does not
// provide it. The internal helpers below share only the implementation of a native lookup. Keeping
// them force-inlined leaves the intrinsic at its original use site, so capability and stage checks
// still apply to the shader that reads the property.

/// Returns the minimum valid distance for the current ray.
[ForceInline]
internal float getStageInputMinDistance()
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

/// Returns the current hit distance or traversal upper bound, depending on the stage.
[ForceInline]
internal float getStageInputCurrentDistance()
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

/// Returns the classification of the current candidate or committed hit.
[ForceInline]
internal uint getStageInputHitKind()
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

/// Returns the flags supplied to the current trace operation.
[ForceInline]
internal uint getStageInputRayFlags()
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

/// Returns the index of the current primitive within its geometry.
[ForceInline]
internal uint getStageInputPrimitiveIndex()
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

/// Returns the index of the current geometry within its primitive acceleration structure.
[ForceInline]
internal uint getStageInputGeometryIndex()
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

/// Returns the origin of the current ray in world space.
[ForceInline]
internal float3 getStageInputWorldSpaceOrigin()
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

/// Returns the direction of the current ray in world space.
[ForceInline]
internal float3 getStageInputWorldSpaceDirection()
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

/// Returns the three-dimensional launch index of the current ray dispatch.
[ForceInline]
internal uint3 getStageInputDispatchRaysIndex()
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

/// Returns the dimensions of the current ray dispatch.
[ForceInline]
internal uint3 getStageInputDispatchRaysDimensions()
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

/// Returns the motion-interpolation time of the current ray.
[ForceInline]
internal float getStageInputRayTime()
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

/// Returns the index of the current instance in its acceleration structure.
[ForceInline]
internal uint getStageInputInstanceIndex()
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

/// Returns the application-provided identifier of the current instance.
[ForceInline]
internal uint getStageInputInstanceID()
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

/// Returns the current candidate ray transformed into object space.
///
/// Any-hit and intersection stages have direct object-ray intrinsics on every portable target.
[ForceInline]
internal RayDesc getStageInputCandidateObjectSpaceRay()
{
    RayDesc result = {};
    __target_switch
    {
    case hlsl:
    case glsl:
    case spirv:
    case cuda:
        result.Origin = ObjectRayOrigin();
        result.TMin = RayTMin();
        result.Direction = ObjectRayDirection();
        result.TMax = RayTCurrent();
    default:
        break;
    }
    return result;
}

/// Returns the committed-hit ray transformed into object space.
///
/// OptiX does not expose object-ray intrinsics in a closest-hit stage, so that target reconstructs
/// the same view from closest-hit-legal world-ray and transform-stack values.
[ForceInline]
internal RayDesc getStageInputClosestHitObjectSpaceRay()
{
    RayDesc result = {};
    __target_switch
    {
    case hlsl:
    case glsl:
    case spirv:
        result.Origin = ObjectRayOrigin();
        result.TMin = RayTMin();
        result.Direction = ObjectRayDirection();
        result.TMax = RayTCurrent();
    case cuda:
        let worldToObject = WorldToObject3x4();
        result.Origin = mul(worldToObject, float4(WorldRayOrigin(), 1.0));
        result.TMin = RayTMin();
        result.Direction = mul(worldToObject, float4(WorldRayDirection(), 0.0));
        result.TMax = RayTCurrent();
    default:
        break;
    }
    return result;
}

/// Returns the transform from the current instance's object space to world space.
[ForceInline]
internal float4x3 getStageInputObjectToWorld()
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

/// Returns the transform from world space to the current instance's object space.
[ForceInline]
internal float4x3 getStageInputWorldToObject()
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
        get { return getStageInputMinDistance(); }
    }

    public property float distance
    {
        get { return getStageInputCurrentDistance(); }
    }

    public property uint hitKind
    {
        get { return getStageInputHitKind(); }
    }

    public property uint rayFlags
    {
        get { return getStageInputRayFlags(); }
    }

    public property uint primitiveIndex
    {
        get { return getStageInputPrimitiveIndex(); }
    }

    public property uint geometryIndex
    {
        get { return getStageInputGeometryIndex(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return getStageInputWorldSpaceOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return getStageInputWorldSpaceDirection(); }
    }

    public property uint3 dispatchRaysIndex
    {
        get { return getStageInputDispatchRaysIndex(); }
    }

    public property uint3 dispatchRaysDimensions
    {
        get { return getStageInputDispatchRaysDimensions(); }
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.TraceContext.Motion : IEnabledRayMotion
{
    public property float time
    {
        get { return getStageInputRayTime(); }
    }
}

public extension<Context> ClosestHitInput<Context>
    where Context : IHitContext
    where Context.TraceContext.AccelerationStructure == AccelerationStructure
{
    public property uint instanceIndex
    {
        get { return getStageInputInstanceIndex(); }
    }

    public property uint instanceID
    {
        get { return getStageInputInstanceID(); }
    }

    public property RayDesc objectSpaceRay
    {
        get { return getStageInputClosestHitObjectSpaceRay(); }
    }

    public property float4x3 objectToWorld
    {
        get { return getStageInputObjectToWorld(); }
    }

    public property float4x3 worldToObject
    {
        get { return getStageInputWorldToObject(); }
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
        get { return getStageInputMinDistance(); }
    }

    public property float distance
    {
        get { return getStageInputCurrentDistance(); }
    }

    public property uint hitKind
    {
        get { return getStageInputHitKind(); }
    }

    public property uint rayFlags
    {
        get { return getStageInputRayFlags(); }
    }

    public property uint primitiveIndex
    {
        get { return getStageInputPrimitiveIndex(); }
    }

    public property uint geometryIndex
    {
        get { return getStageInputGeometryIndex(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return getStageInputWorldSpaceOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return getStageInputWorldSpaceDirection(); }
    }

    public property uint3 dispatchRaysIndex
    {
        get { return getStageInputDispatchRaysIndex(); }
    }

    public property uint3 dispatchRaysDimensions
    {
        get { return getStageInputDispatchRaysDimensions(); }
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
        get { return getStageInputRayTime(); }
    }
}

public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.TraceContext.AccelerationStructure == AccelerationStructure
{
    public property uint instanceIndex
    {
        get { return getStageInputInstanceIndex(); }
    }

    public property uint instanceID
    {
        get { return getStageInputInstanceID(); }
    }

    public property RayDesc objectSpaceRay
    {
        get { return getStageInputCandidateObjectSpaceRay(); }
    }

    public property float4x3 objectToWorld
    {
        get { return getStageInputObjectToWorld(); }
    }

    public property float4x3 worldToObject
    {
        get { return getStageInputWorldToObject(); }
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
/// `reportHit` submits each candidate and its `Context.Primitive.Attributes` value. On D3D,
/// Vulkan, and OptiX it maps to the native report-intersection operation; Metal lowering preserves
/// the same candidate-filtering behavior in the generated intersection function.
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
        get { return getStageInputMinDistance(); }
    }

    public property uint rayFlags
    {
        get { return getStageInputRayFlags(); }
    }

    public property RayDesc objectSpaceRay
    {
        get { return getStageInputCandidateObjectSpaceRay(); }
    }

    public property uint primitiveIndex
    {
        get { return getStageInputPrimitiveIndex(); }
    }

    public property uint geometryIndex
    {
        get { return getStageInputGeometryIndex(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return getStageInputWorldSpaceOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return getStageInputWorldSpaceDirection(); }
    }

    public property uint3 dispatchRaysIndex
    {
        get { return getStageInputDispatchRaysIndex(); }
    }

    public property uint3 dispatchRaysDimensions
    {
        get { return getStageInputDispatchRaysDimensions(); }
    }

    public bool reportHit(float distance, Context.Primitive.Attributes attributes)
    {
        __target_switch
        {
        case hlsl:
        case glsl:
        case spirv:
        case cuda:
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
        case cuda:
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
        get { return getStageInputRayTime(); }
    }
}

public extension<Context> IntersectionInput<Context>
    where Context : IHitContext
    where Context.TraceContext.AccelerationStructure == AccelerationStructure
{
    public property uint instanceIndex
    {
        get { return getStageInputInstanceIndex(); }
    }

    public property uint instanceID
    {
        get { return getStageInputInstanceID(); }
    }

    public property float4x3 objectToWorld
    {
        get { return getStageInputObjectToWorld(); }
    }

    public property float4x3 worldToObject
    {
        get { return getStageInputWorldToObject(); }
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
        get { return getStageInputMinDistance(); }
    }

    public property float distance
    {
        get { return getStageInputCurrentDistance(); }
    }

    public property uint rayFlags
    {
        get { return getStageInputRayFlags(); }
    }

    public property float3 worldSpaceOrigin
    {
        get { return getStageInputWorldSpaceOrigin(); }
    }

    public property float3 worldSpaceDirection
    {
        get { return getStageInputWorldSpaceDirection(); }
    }

    public property uint3 dispatchRaysIndex
    {
        get { return getStageInputDispatchRaysIndex(); }
    }

    public property uint3 dispatchRaysDimensions
    {
        get { return getStageInputDispatchRaysDimensions(); }
    }
}

public extension<Context> MissInput<Context>
    where Context : IPayloadContext
    where Context.TraceContext.Motion : IEnabledRayMotion
{
    public property float time
    {
        get { return getStageInputRayTime(); }
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
        get { return getStageInputDispatchRaysIndex(); }
    }

    public property uint3 dispatchRaysDimensions
    {
        get { return getStageInputDispatchRaysDimensions(); }
    }
}

} // namespace rt
