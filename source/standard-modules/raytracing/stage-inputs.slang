implementing raytracing;

namespace rt
{

// Stage inputs are compiler-provided views of the current invocation, with no source storage.
// Native targets expose much of that state through existing core-module intrinsics. The helpers
// remain force-inlined so capability and stage checks apply at the shader's property access.
//
// Metal stage-input lowering must obtain these values from generated intersection or
// post-traversal dispatch parameters. Each pending Metal implementation therefore fails with a
// target-specific static assertion until that synthesis is added in a following PR. These
// assertions allow module import and frontend checking; they reject reachable target code
// generation. Returns after a failed assertion only satisfy return checking and cannot execute.

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
    case metal:
        // TODO: A following PR must synthesize the ray minimum distance from Metal stage state.
        static_assert(
            false,
            "Metal structural ray-tracing minDistance requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the current hit distance or traversal upper bound
        // from Metal stage state.
        static_assert(false, "Metal structural ray-tracing distance requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the candidate or committed hit classification from
        // Metal stage state.
        static_assert(false, "Metal structural ray-tracing hitKind requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the trace call's ray flags from Metal stage state.
        static_assert(false, "Metal structural ray-tracing rayFlags requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the candidate or committed primitive index from
        // Metal stage state.
        static_assert(
            false,
            "Metal structural ray-tracing primitiveIndex requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the candidate or committed geometry index from Metal
        // stage state.
        static_assert(
            false,
            "Metal structural ray-tracing geometryIndex requires compiler synthesis");
        return 0;
    case cuda:
        // TODO: A following PR must implement the geometry-index mapping for OptiX.
        static_assert(false, "CUDA structural ray-tracing geometryIndex is not implemented");
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
    case metal:
        // TODO: A following PR must synthesize the world-space ray origin from Metal stage state.
        static_assert(
            false,
            "Metal structural ray-tracing worldSpaceOrigin requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the world-space ray direction from Metal stage
        // state.
        static_assert(
            false,
            "Metal structural ray-tracing worldSpaceDirection requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the originating dispatch index from Metal stage
        // state.
        static_assert(
            false,
            "Metal structural ray-tracing dispatchRaysIndex requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the originating dispatch dimensions from Metal stage
        // state.
        static_assert(
            false,
            "Metal structural ray-tracing dispatchRaysDimensions requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the motion-enabled ray time from Metal stage state.
        static_assert(false, "Metal structural ray-tracing time requires compiler synthesis");
        return 0.0;
    case cuda:
        // TODO: A following PR must expose the current OptiX ray time through the core module.
        static_assert(false, "CUDA structural ray-tracing time is not implemented");
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
    case metal:
        // TODO: A following PR must synthesize the candidate or committed instance index from Metal
        // stage state.
        static_assert(
            false,
            "Metal structural ray-tracing instanceIndex requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the application-provided instance identifier from
        // Metal stage state.
        static_assert(false, "Metal structural ray-tracing instanceID requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the candidate ray in object space from Metal stage
        // state.
        static_assert(
            false,
            "Metal structural ray-tracing objectSpaceRay requires compiler synthesis");
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
    case metal:
        // TODO: A following PR must synthesize the committed-hit ray in object space from Metal
        // stage state.
        static_assert(
            false,
            "Metal structural ray-tracing objectSpaceRay requires compiler synthesis");
        break;
    }
    return result;
}

/// Returns the transform from the hit geometry's object space to the traversal's world space.
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
    case metal:
        // TODO: A following PR must synthesize the complete object-to-world transform from Metal
        // stage state, or an identity matrix for direct primitive-AS traversal without instancing.
        static_assert(
            false,
            "Metal structural ray-tracing objectToWorld requires compiler synthesis");
        return 0.0;
    }
}

/// Returns the transform from the traversal's world space to the hit geometry's object space.
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
    case metal:
        // TODO: A following PR must synthesize the complete world-to-object transform from Metal
        // stage state, or an identity matrix for direct primitive-AS traversal without instancing.
        static_assert(
            false,
            "Metal structural ray-tracing worldToObject requires compiler synthesis");
        return 0.0;
    }
}

/// Identifies the stage views that share ray and geometry state.
///
/// This internal contract shares implementations without introducing an erased public stage-input
/// type. Concrete input types retain their stage restrictions and context-dependent properties.
///
/// The shared property bodies live in the public extension below because Slang currently rejects
/// default property accessor bodies in interfaces and public members of an internal interface.
/// For example, putting the public `rayFlags` getter here would violate both rules. The extension
/// exposes `input.rayFlags` on each concrete input while this implementation-sharing marker remains
/// internal.
internal interface ICommonHitInput
{}

/// Provides the properties available to closest-hit, any-hit, and intersection stages.
///
/// An intersection stage has not reported a hit yet, so `distance` and `hitKind` stay on the
/// closest-hit and any-hit inputs. Motion and scalar instance identifiers remain on their
/// constrained concrete extensions.
public extension<Input : ICommonHitInput> Input
{
    public property float minDistance
    {
        get { return getStageInputMinDistance(); }
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

    /// Transforms the hit geometry's object-space coordinates into the traversal's world space.
    ///
    /// This includes every instance transform along the hit path. For example, an inner instance
    /// translating by two units and an outer instance translating by ten units on the same axis
    /// produce a combined translation of twelve units. Without instancing, this is the identity.
    public property float4x3 objectToWorld
    {
        get { return getStageInputObjectToWorld(); }
    }

    /// Transforms world-space coordinates into the hit geometry's object space.
    ///
    /// This is the inverse of the complete `objectToWorld` transform, including all instance
    /// levels. Without instancing, this is the identity.
    public property float4x3 worldToObject
    {
        get { return getStageInputWorldToObject(); }
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

/// Provides the payload, record, hit data, and ray state available to a closest-hit stage.
///
/// This is a compiler-provided zero-storage view. Adapter synthesis must map its properties to
/// native stage parameters or built-in state without adding source fields to the entry point.
[require(structural_raytracing_closesthit)]
public struct ClosestHitInput<Context> : ICommonHitInput
    where Context : IHitContext
{
    /// Gives mutable access to the payload supplied by the originating `RayTracer.trace` call.
    /// TODO: The compiler must synthesize this accessor from the stage's payload parameter.
    public property Context.Payload payload
    {
        ref;
    }

    /// Reads the host-provided data in the selected shader-binding-table record.
    /// TODO: The compiler must synthesize this accessor from the target's record-data binding.
    public property Context.Record record
    {
        get;
    }

    /// Reads the committed hit's attributes using the type selected by `Context.Primitive`.
    ///
    /// A triangle context exposes `attributes.barycentricCoord` through `TriangleData`; a
    /// `BoundingBoxPrimitive<MyAttributes>` context exposes the `MyAttributes` value reported by
    /// its intersection shader. A curve context exposes `CurveData`. Generic callers need ordinary
    /// type constraints before accessing members of the associated attribute type.
    /// TODO: Adapter synthesis must return the built-in view or read the procedural hit attributes.
    public property Context.Primitive.Attributes attributes
    {
        get;
    }

    public property float distance
    {
        get { return getStageInputCurrentDistance(); }
    }

    public property uint hitKind
    {
        get { return getStageInputHitKind(); }
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
}

/// Provides the candidate payload, record, hit data, and ray state available to an any-hit stage.
///
/// Control methods on this zero-storage view use native any-hit operations where available.
/// TODO: Metal adapter synthesis must implement them in the generated candidate dispatch.
[require(structural_raytracing_anyhit)]
public struct AnyHitInput<Context> : ICommonHitInput
    where Context : IHitContext
{
    /// Gives mutable access to the payload supplied by the originating `RayTracer.trace` call.
    /// TODO: The compiler must synthesize this accessor from the stage's payload parameter.
    public property Context.Payload payload
    {
        ref;
    }

    /// Reads the host-provided data in the selected shader-binding-table record.
    /// TODO: The compiler must synthesize this accessor from the target's record-data binding.
    public property Context.Record record
    {
        get;
    }

    /// Reads the candidate hit's attributes using the type selected by `Context.Primitive`.
    ///
    /// Built-in primitives expose their `TriangleData` or `CurveData` view; procedural primitives
    /// expose the application-defined value supplied by the intersection shader's `reportHit` call.
    /// TODO: Adapter synthesis must return the built-in view or read the procedural hit attributes.
    public property Context.Primitive.Attributes attributes
    {
        get;
    }

    public property float distance
    {
        get { return getStageInputCurrentDistance(); }
    }

    public property uint hitKind
    {
        get { return getStageInputHitKind(); }
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
        case metal:
            // TODO: A following PR must synthesize Metal candidate rejection.
            static_assert(
                false,
                "Metal structural ray-tracing ignoreHit requires compiler synthesis");
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
        case metal:
            // TODO: A following PR must synthesize Metal candidate acceptance and traversal
            // termination.
            static_assert(
                false,
                "Metal structural ray-tracing acceptHitAndEndSearch requires compiler synthesis");
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
}

/// Provides the record and ray state used by a procedural intersection stage.
///
/// `reportHit` submits each candidate and its `Context.Primitive.Attributes` value. On D3D,
/// Vulkan, and OptiX it maps to the native report-intersection operation.
/// TODO: Metal adapter synthesis must preserve the same candidate-filtering behavior in the
/// generated intersection function.
[require(structural_raytracing_intersection)]
public struct IntersectionInput<Context> : ICommonHitInput
    where Context : IHitContext
{
    /// Reads the host-provided data in the selected shader-binding-table record.
    /// TODO: The compiler must synthesize this accessor from the target's record-data binding.
    public property Context.Record record
    {
        get;
    }

    public property RayDesc objectSpaceRay
    {
        get { return getStageInputCandidateObjectSpaceRay(); }
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
        case metal:
            // TODO: A following PR must synthesize Metal procedural candidate reporting and any-hit
            // dispatch.
            static_assert(
                false,
                "Metal structural ray-tracing reportHit requires compiler synthesis");
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
        case metal:
            // TODO: A following PR must synthesize Metal procedural candidate reporting and any-hit
            // dispatch.
            static_assert(
                false,
                "Metal structural ray-tracing reportHit requires compiler synthesis");
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
}

/// Provides the payload, record, and ray state available to a miss stage.
///
/// Adapter synthesis must map this zero-storage view to native miss parameters where available
/// and to Metal's generated post-traversal dispatch.
[require(structural_raytracing_miss)]
public struct MissInput<Context>
    where Context : IPayloadContext
{
    /// Gives mutable access to the payload supplied by the originating `RayTracer.trace` call.
    /// TODO: The compiler must synthesize this accessor from the stage's payload parameter.
    public property Context.Payload payload
    {
        ref;
    }

    /// Reads the host-provided data in the selected shader-binding-table record.
    /// TODO: The compiler must synthesize this accessor from the target's record-data binding.
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
    /// Gives mutable access to the data supplied by the originating `RayTracer.callShader` call.
    /// TODO: The compiler must synthesize this accessor from the callable-data parameter.
    public property Context.CallableData data
    {
        ref;
    }

    /// Reads the host-provided data in the selected shader-binding-table record.
    /// TODO: The compiler must synthesize this accessor from the target's record-data binding.
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
