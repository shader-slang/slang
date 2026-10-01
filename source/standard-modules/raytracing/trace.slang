implementing raytracing;

namespace rt
{

/// Provides schema-typed structural trace and callable dispatch operations.
///
/// Extensions below define the source contract selected by the schema's acceleration-structure
/// topology and motion model. Later lowering will map that contract to a target ABI.
public struct RayTracer<Schema>
    where Schema : ITraceProgramSchema
{}

/// Traces a native two-level acceleration structure without motion.
///
/// The intended portable lowering maps the payload overload to `TraceRay`; the intended Metal
/// lowering consumes the same call as a structural operation and emits intersector dispatch.
[require(structural_raytracing_trace)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == NoMotion
{
    /// Traces with the schema's unique empty payload on the native no-motion ABI.
    [ForceInline]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with caller-owned payload storage shared by the reachable hit and miss stages.
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
[require(structural_raytracing_trace, metallib_2_4)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    /// Traces with the schema's unique empty payload and Metal primitive-motion traversal.
    [ForceInline]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload and Metal primitive-motion traversal.
    [ForceInline]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a native two-level acceleration structure with instance motion.
///
/// The intended Vulkan and OptiX lowering maps the payload overload to `TraceMotionRay`; the
/// intended Metal lowering emits the equivalent intersector configuration.
[require(structural_raytracing_trace, structural_raytracing_instance_motion)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == InstanceMotion
{
    /// Traces with the schema's unique empty payload and native instance-motion traversal.
    [ForceInline]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload and native instance-motion traversal.
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
[require(structural_raytracing_trace, metallib_2_4)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    /// Traces with the schema's unique empty payload and combined Metal motion traversal.
    [ForceInline]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // TODO: Select the schema's unique empty payload and supply its storage during lowering.
    }

    /// Traces with a mutable payload and combined Metal motion traversal.
    [ForceInline]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure without motion.
///
/// The method-level equality binds the handle's `maxLevelCount` to the schema topology while
/// keeping the extension reusable for every supported depth.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == NoMotion
{
    /// Traces with the schema's unique empty payload and a depth inferred from the scene handle.
    [ForceInline]
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
    [ForceInline]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with primitive motion.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and primitive motion.
    [ForceInline]
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
    [ForceInline]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with instance motion.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == InstanceMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and instance motion.
    [ForceInline]
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
    [ForceInline]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // TODO: Lower this structural operation to Metal intersector dispatch before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with primitive and instance motion.
[require(structural_raytracing_trace, metal)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and combined motion.
    [ForceInline]
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
    [ForceInline]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
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
[require(structural_raytracing_call_shader)]
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
{
    /// Invokes the callable record at `callableIndex` with context-typed mutable data.
    ///
    /// `CallableContext.TraceContext` must match the schema so the selected callable stage observes
    /// the same trace-wide contract as the descriptor.
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
            break;
        default:
            CallShader(callableIndex, data);
        }
    }
}

} // namespace rt
