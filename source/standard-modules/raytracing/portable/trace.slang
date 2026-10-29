implementing portable;

namespace rt
{

/// Supplies the local empty value used by trace overloads without a payload argument.
///
/// The wrapper passes this value to the ordinary payload overload. For example,
/// `tracer.trace(desc, scene, program)` forwards an `EmptyTracePayload` as its fourth argument.
/// Application shaders can keep their own empty payload types; they do not name this internal type.
/// TODO: Post-link lowering must normalize empty payload representations across trace calls and
/// their reached stages before generating native code.
internal struct EmptyTracePayload
{}

/// Emits the Metal trace operation for traversal without motion.
///
/// The ordinary intrinsic-call lowering passes these arguments directly to `metalTraceRay`.
/// Its absent time operand selects no motion. The descriptor and payload pointer retain their
/// checked types, so lowering does not need to recover generic arguments from a function name.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @tparam Payload Mutable payload type used by the selected hit and miss programs.
/// @param[in] accelerationStructure Handle of exactly `Schema.TraceContext.AccelerationStructure`.
/// @param[in] descriptor Program descriptor whose type retains the schema and its conformance.
/// @param[in] rayFlags Traversal and hit-shader control flags.
/// @param[in] instanceMask Mask used to select acceleration-structure instances.
/// @param[in] sbtOffset Offset of the selected ray type within each geometry's hit records.
/// @param[in] sbtStride Number of hit records per geometry, measured in records rather than bytes.
/// @param[in] missIndex Index of the selected miss record.
/// @param[in] rayOrigin Ray origin in world space.
/// @param[in] rayTMin Minimum accepted ray distance.
/// @param[in] rayDirection Ray direction in world space.
/// @param[in] rayTMax Maximum accepted ray distance.
/// @param[in,out] payload Payload storage read and updated by the selected shaders.
/// @par Constraints
/// Requires a Metal trace stage and `Schema.TraceContext.Motion == NoMotion`.
/// @todo Lower this operation to Metal intersector dispatch in the backend adapter slice.
[require(metal, structural_raytracing_trace)]
__intrinsic_op(metalTraceRay)
internal void __metalTraceRay<Schema, Payload>(
    Schema.TraceContext.AccelerationStructure accelerationStructure,
    TraceProgramDescriptor<Schema> descriptor,
    uint rayFlags,
    uint instanceMask,
    uint sbtOffset,
    uint sbtStride,
    uint missIndex,
    float3 rayOrigin,
    float rayTMin,
    float3 rayDirection,
    float rayTMax,
    inout Payload payload)
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == NoMotion;

/// Emits the same Metal trace operation with an explicit motion-sampling time.
///
/// The final time operand distinguishes this operation from a no-motion trace, even when time
/// is zero. The descriptor's schema retains whether primitives, instances, or both can move.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @tparam Payload Mutable payload type used by the selected hit and miss programs.
/// @param[in] accelerationStructure Handle of exactly `Schema.TraceContext.AccelerationStructure`.
/// @param[in] descriptor Program descriptor whose type retains the schema and its conformance.
/// @param[in] rayFlags Traversal and hit-shader control flags.
/// @param[in] instanceMask Mask used to select acceleration-structure instances.
/// @param[in] sbtOffset Offset of the selected ray type within each geometry's hit records.
/// @param[in] sbtStride Number of hit records per geometry, measured in records rather than bytes.
/// @param[in] missIndex Index of the selected miss record.
/// @param[in] rayOrigin Ray origin in world space.
/// @param[in] rayTMin Minimum accepted ray distance.
/// @param[in] rayDirection Ray direction in world space.
/// @param[in] rayTMax Maximum accepted ray distance.
/// @param[in,out] payload Payload storage read and updated by the selected shaders.
/// @param[in] time Time at which traversal samples moving geometry and instance transforms.
/// @par Constraints
/// Requires Metal 2.4 and a trace stage. `Schema.TraceContext.Motion : IEnabledRayMotion`
/// admits primitive motion, instance motion, and their combination.
/// @todo Lower this operation to Metal intersector dispatch in the backend adapter slice.
[require(metallib_2_4, structural_raytracing_trace)]
__intrinsic_op(metalTraceRay)
internal void __metalTraceRayMotion<Schema, Payload>(
    Schema.TraceContext.AccelerationStructure accelerationStructure,
    TraceProgramDescriptor<Schema> descriptor,
    uint rayFlags,
    uint instanceMask,
    uint sbtOffset,
    uint sbtStride,
    uint missIndex,
    float3 rayOrigin,
    float rayTMin,
    float3 rayDirection,
    float rayTMax,
    inout Payload payload,
    float time)
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion : IEnabledRayMotion;

/// Provides schema-typed structural trace and callable dispatch operations.
///
/// No-motion and motion-enabled traversal each have overloads with and without a payload.
/// The three enabled motion types share one extension; the schema retains the exact motion
/// choice for target lowering. Both extensions take the schema's `AccelerationStructure` type,
/// so native and multilevel handles share the same overloads.
/// `ITraceContext` requires that type to conform to `IAccelerationStructure`;
/// `MultiLevelAccelerationStructure<N>` declares its Metal requirement on the type itself.
/// Later lowering will map each call to the target's tracing operation.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
public struct RayTracer<Schema>
    where Schema : ITraceProgramSchema
{}

/// Traces the schema's acceleration structure without motion.
///
/// The intended portable lowering maps the payload overload to `TraceRay`; the intended Metal
/// lowering consumes the same call as a structural operation and emits intersector dispatch.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.Motion == NoMotion` disables motion during traversal.
[require(structural_raytracing_trace)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == NoMotion
{
    /// Traces without application payload data or motion.
    ///
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` is unused because this overload requires `NoMotion`.
    /// @param[in] accelerationStructure Handle of exactly `Schema.TraceContext.AccelerationStructure`.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    [ForceInline]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        EmptyTracePayload payload = {};
        trace(desc, accelerationStructure, descriptor, payload);
    }

    /// Traces with caller-owned payload storage shared by the reachable hit and miss stages.
    ///
    /// @tparam Payload Payload type shared with reachable hit and miss stages.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` is unused because this overload requires `NoMotion`.
    /// @param[in] accelerationStructure Handle of exactly `Schema.TraceContext.AccelerationStructure`.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    [ForceInline]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        __target_switch
        {
        case metal:
            __metalTraceRay<Schema, Payload>(
                accelerationStructure,
                descriptor,
                desc.rayFlags,
                desc.instanceMask,
                desc.sbtOffset,
                desc.sbtStride,
                desc.missIndex,
                desc.ray.Origin,
                desc.ray.TMin,
                desc.ray.Direction,
                desc.ray.TMax,
                payload);
            break;
        default:
            // These targets support only AccelerationStructure. The cast lets this generic
            // body call TraceRay's concrete-handle overload; a supported call casts the
            // handle to its own type.
            TraceRay(
                bit_cast<AccelerationStructure>(accelerationStructure),
                desc.rayFlags,
                desc.instanceMask,
                desc.sbtOffset,
                desc.sbtStride,
                desc.missIndex,
                desc.ray,
                payload);
        }
    }
}

/// Traces the schema's acceleration structure at the supplied ray time.
///
/// The intended Vulkan and OptiX lowering maps instance-motion calls to `TraceMotionRay`.
/// Metal lowering reads `Schema.TraceContext.Motion` to select primitive motion, instance
/// motion, or both. Sharing this method preserves that choice in the schema type.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.Motion : IEnabledRayMotion` admits `PrimitiveMotion`, `InstanceMotion`,
///   and `PrimitiveAndInstanceMotion`. `NoMotion` uses the other extension.
/// The selected motion type retains its own target requirements: primitive and combined motion
/// require Metal 2.4, while instance motion also supports native motion-blur targets.
[require(structural_raytracing_trace, structural_raytracing_instance_motion)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion : IEnabledRayMotion
{
    /// Traces without application payload data at the time given by `desc.time`.
    ///
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Handle of exactly `Schema.TraceContext.AccelerationStructure`.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    [ForceInline]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        EmptyTracePayload payload = {};
        trace(desc, accelerationStructure, descriptor, payload);
    }

    /// Traces with a mutable payload at the time given by `desc.time`.
    ///
    /// @tparam Payload Payload type shared with reachable hit and miss stages.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Handle of exactly `Schema.TraceContext.AccelerationStructure`.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    [ForceInline]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        __target_switch
        {
        case metal:
            __metalTraceRayMotion<Schema, Payload>(
                accelerationStructure,
                descriptor,
                desc.rayFlags,
                desc.instanceMask,
                desc.sbtOffset,
                desc.sbtStride,
                desc.missIndex,
                desc.ray.Origin,
                desc.ray.TMin,
                desc.ray.Direction,
                desc.ray.TMax,
                payload,
                desc.time);
            break;
        case cuda:
            // TODO: Add the timed OptiX path to the core TraceMotionRay intrinsic, then reuse it
            // here as on GLSL/SPIR-V. The current core intrinsic only supports those two targets;
            // the CUDA TraceRay prelude wrapper fixes ray time to zero and cannot implement this.
            static_assert(false, "CUDA structural ray-tracing motion is not implemented");
            break;
        case glsl:
        case spirv:
            // On these targets the supported handle is AccelerationStructure, so the cast
            // connects the generic parameter to TraceMotionRay's concrete signature.
            TraceMotionRay(
                bit_cast<AccelerationStructure>(accelerationStructure),
                desc.rayFlags,
                desc.instanceMask,
                desc.sbtOffset,
                desc.sbtStride,
                desc.missIndex,
                desc.ray,
                desc.time,
                payload);
        default:
            break;
        }
    }
}

/// Dispatches a callable record from the schema's callable section.
///
/// The intended portable lowering maps this operation to `CallShader`; the intended Metal lowering
/// maps it to visible-function-table dispatch.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
[require(structural_raytracing_call_shader)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
{
    /// Invokes the callable record at `callableIndex` with context-typed mutable data.
    ///
    /// @tparam CallableContext Callable-stage context; must conform to `ICallableContext`.
    /// @param[in] callableIndex Index of the runtime callable record to invoke in `descriptor`.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] data Callable data of exactly `CallableContext.CallableData`; the selected
    /// callable stage reads and updates this storage.
    /// @par Constraints
    /// `CallableContext : ICallableContext` supplies the callable-data type and trace context.
    /// `CallableContext.TraceContext == Schema.TraceContext` requires the callable stage and the
    /// descriptor to share the same traversal contract.
    [ForceInline]
    public void callShader<CallableContext>(
        uint callableIndex,
        TraceProgramDescriptor<Schema> descriptor,
        inout CallableContext.CallableData data)
        where CallableContext : ICallableContext
        where CallableContext.TraceContext == Schema.TraceContext
    {
        __target_switch
        {
        case metal:
            // TODO: Lower this structural dispatch to a Metal visible-function-table call.
            static_assert(
                false,
                "Metal structural ray-tracing callable dispatch requires compiler synthesis");
            break;
        default:
            CallShader(callableIndex, data);
        }
    }
}

} // namespace rt
