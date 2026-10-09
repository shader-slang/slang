implementing raytracing;

namespace rt
{

/// Provides schema-typed structural trace and callable dispatch operations.
///
/// Extensions below define the source contract selected by the schema's acceleration-structure
/// topology and motion model. Later lowering will map that contract to a target ABI.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
__magic_type(RayTracerType)
public struct RayTracer<Schema>
    where Schema : ITraceProgramSchema
{}

/// Traces a native two-level acceleration structure without motion.
///
/// The intended portable lowering maps the payload overload to `TraceRay`; the intended Metal
/// lowering consumes the same call as a structural operation and emits intersector dispatch.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.AccelerationStructure == AccelerationStructure` selects the native
///   two-level handle.
/// - `Schema.TraceContext.Motion == NoMotion` disables motion during traversal.
[require(structural_raytracing_trace)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == NoMotion
{
    /// Traces with the schema's unique empty payload on the native no-motion ABI.
    ///
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` is unused because this overload requires `NoMotion`.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    [ForceInline]
    [__RayTracingTrace]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with caller-owned payload storage shared by the reachable hit and miss stages.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` is unused because this overload requires `NoMotion`.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
    {
        __target_switch
        {
        case metal:
            // TODO: Lower this structural operation to Metal intersector dispatch before emission.
            break;
        default:
            TraceRay(
                accelerationStructure,
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

/// Traces a native two-level acceleration structure with primitive motion on Metal.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.AccelerationStructure == AccelerationStructure` selects the native
///   two-level handle.
/// - `Schema.TraceContext.Motion == PrimitiveMotion` selects motion of geometry within a primitive
///   acceleration structure.
[require(structural_raytracing_trace, metallib_2_4)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    /// Traces with the schema's unique empty payload and Metal primitive-motion traversal.
    ///
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    [ForceInline]
    [__RayTracingTrace]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload and Metal primitive-motion traversal.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a native two-level acceleration structure with instance motion.
///
/// The intended Vulkan and OptiX lowering maps the payload overload to `TraceMotionRay`; the
/// intended Metal lowering emits the equivalent intersector configuration.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.AccelerationStructure == AccelerationStructure` selects the native
///   two-level handle.
/// - `Schema.TraceContext.Motion == InstanceMotion` selects motion of instance transforms.
[require(structural_raytracing_trace, structural_raytracing_instance_motion)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == InstanceMotion
{
    /// Traces with the schema's unique empty payload and native instance-motion traversal.
    ///
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    [ForceInline]
    [__RayTracingTrace]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload and native instance-motion traversal.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
    {
        __target_switch
        {
        case metal:
            // TODO: Lower this structural operation to Metal intersector dispatch before emission.
            break;
        case cuda:
            // TODO: Add the timed OptiX path to the core TraceMotionRay intrinsic, then reuse it
            // here as on GLSL/SPIR-V. The current core intrinsic only supports those two targets;
            // the CUDA TraceRay prelude wrapper fixes ray time to zero and cannot implement this.
            break;
        case glsl:
        case spirv:
            TraceMotionRay(
                accelerationStructure,
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

/// Traces a native two-level acceleration structure with primitive and instance motion on Metal.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.AccelerationStructure == AccelerationStructure` selects the native
///   two-level handle.
/// - `Schema.TraceContext.Motion == PrimitiveAndInstanceMotion` selects both geometry
///   motion and motion of instance transforms.
[require(structural_raytracing_trace, metallib_2_4)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    /// Traces with the schema's unique empty payload and combined Metal motion traversal.
    ///
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    [ForceInline]
    [__RayTracingTrace]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload and combined Metal motion traversal.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Native two-level handle matching the schema's
    /// acceleration-structure type.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure without motion.
///
/// The method-level equality binds the handle's `maxLevelCount` to the schema topology while
/// keeping the extension reusable for every supported depth.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.Motion == NoMotion` disables motion during traversal.
///
/// The `trace` methods additionally require the schema's acceleration-structure type to equal
/// `MultiLevelAccelerationStructure<maxLevelCount>`; their method constraints select that topology.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == NoMotion
{
    /// Traces with the schema's unique empty payload and a depth inferred from the scene handle.
    ///
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` is unused because this overload requires `NoMotion`.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload and a depth inferred from the scene handle.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` is unused because this overload requires `NoMotion`.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with primitive motion.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.Motion == PrimitiveMotion` selects motion of geometry within a primitive
///   acceleration structure.
///
/// The `trace` methods additionally require the schema's acceleration-structure type to equal
/// `MultiLevelAccelerationStructure<maxLevelCount>`; their method constraints select that topology.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and primitive motion.
    ///
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload, explicit depth, and primitive motion.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with instance motion.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.Motion == InstanceMotion` selects motion of instance transforms.
///
/// The `trace` methods additionally require the schema's acceleration-structure type to equal
/// `MultiLevelAccelerationStructure<maxLevelCount>`; their method constraints select that topology.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == InstanceMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and instance motion.
    ///
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload, explicit depth, and instance motion.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with primitive and instance motion.
///
/// @tparam Schema Trace-program schema; must conform to `ITraceProgramSchema`.
/// @par Specialization
/// Extends `RayTracer<Schema>` with these constraints:
/// - `Schema : ITraceProgramSchema` supplies the trace context and shader lists.
/// - `Schema.TraceContext.Motion == PrimitiveAndInstanceMotion` selects both geometry
///   motion and motion of instance transforms.
///
/// The `trace` methods additionally require the schema's acceleration-structure type to equal
/// `MultiLevelAccelerationStructure<maxLevelCount>`; their method constraints select that topology.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and combined motion.
    ///
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload, explicit depth, and combined motion.
    ///
    /// @tparam Payload Non-empty payload type shared with reachable hit and miss stages.
    /// @tparam maxLevelCount Compile-time integer maximum acceleration-structure depth, including
    /// the final primitive acceleration structure. Inferred from `accelerationStructure`.
    /// @param[in] desc Ray, traversal flags, instance mask, and shader-record selectors.
    /// `desc.time` selects the time used for motion during traversal.
    /// @param[in] accelerationStructure Metal handle whose depth and type match the schema.
    /// @param[in] descriptor Program and shader-record descriptor for the same `Schema`.
    /// @param[in,out] payload Payload storage read and updated by the reached hit or miss stages.
    /// @par Constraints
    /// `Schema.TraceContext.AccelerationStructure` must equal
    /// `MultiLevelAccelerationStructure<maxLevelCount>`, so the argument and schema use the same
    /// depth-parameterized handle type.
    [ForceInline]
    [__RayTracingTrace]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        [__RayTracingPayload] inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
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
    [__RayTracingCallShader]
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
            break;
        default:
            CallShader(callableIndex, data);
        }
    }
}

} // namespace rt
