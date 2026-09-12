implementing raytracing;

namespace rt
{

/// Provides schema-typed structural trace and callable dispatch operations.
///
/// Extensions below select the source implementation and target ABI from the schema's
/// acceleration-structure topology and motion contract.
public struct RayTracer<Schema>
    where Schema : ITraceProgramSchema
{}

/// Traces a native two-level acceleration structure without motion.
///
/// Portable targets lower the payload overload to `TraceRay`; Metal consumes the same call as a
/// structural operation and emits its intersector dispatch.
public extension<Schema> RayTracer<Schema>
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == NoMotion
    // Constraint order is not part of the API. Keep the schema conformance after the equality
    // witnesses so compiler-owned discovery must identify it by semantic role.
    where Schema : ITraceProgramSchema
{
    /// Traces with the schema's unique empty payload on the native no-motion ABI.
    [ForceInline]
    [require(structural_raytracing_trace)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with caller-owned payload storage shared by the reachable hit and miss stages.
    [ForceInline]
    [require(structural_raytracing_trace)]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        __target_switch
        {
        case metal:
            // The compiler consumes this call as a structural trace operation before emission.
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
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    /// Traces with the schema's unique empty payload and Metal primitive-motion traversal.
    [ForceInline]
    [require(structural_raytracing_trace, metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with a mutable payload and Metal primitive-motion traversal.
    [ForceInline]
    [require(structural_raytracing_trace, metallib_2_4)]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

/// Traces a native two-level acceleration structure with instance motion.
///
/// Portable targets lower the payload overload to `TraceMotionRay`; Metal emits the equivalent
/// intersector configuration from the structural operation.
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == InstanceMotion
{
    /// Traces with the schema's unique empty payload and native instance-motion traversal.
    [ForceInline]
    [require(structural_raytracing_trace, structural_raytracing_instance_motion)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with a mutable payload and native instance-motion traversal.
    [ForceInline]
    [require(structural_raytracing_trace, structural_raytracing_instance_motion)]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        __target_switch
        {
        case metal:
            // The compiler consumes this call as a structural trace operation before emission.
            break;
        default:
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
        }
    }
}

/// Traces a native two-level acceleration structure with primitive and instance motion on Metal.
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    /// Traces with the schema's unique empty payload and combined Metal motion traversal.
    [ForceInline]
    [require(structural_raytracing_trace, metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with a mutable payload and combined Metal motion traversal.
    [ForceInline]
    [require(structural_raytracing_trace, metallib_2_4)]
    public void trace<Payload>(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

/// Traces a Metal multi-level acceleration structure without motion.
///
/// The method-level equality binds the handle's `maxLevelCount` to the schema topology while
/// keeping the extension reusable for every supported depth.
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == NoMotion
{
    /// Traces with the schema's unique empty payload and a depth inferred from the scene handle.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with a mutable payload and a depth inferred from the scene handle.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with primitive motion.
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and primitive motion.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with a mutable payload, explicit depth, and primitive motion.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with instance motion.
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == InstanceMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and instance motion.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with a mutable payload, explicit depth, and instance motion.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

/// Traces a Metal multi-level acceleration structure with primitive and instance motion.
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    /// Traces with the schema's unique empty payload, explicit depth, and combined motion.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler selects the schema's unique empty payload and supplies its storage.
    }

    /// Traces with a mutable payload, explicit depth, and combined motion.
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<Payload, let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

/// Dispatches a callable record from the schema's callable section.
///
/// Portable targets lower the operation to `CallShader`; Metal consumes it as structural visible
/// function-table dispatch.
public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
{
    /// Invokes the callable record at `callableIndex` with context-typed mutable data.
    ///
    /// `CallableContext.TraceContext` must match the schema so the selected callable stage observes
    /// the same trace-wide contract as the descriptor.
    [ForceInline]
    [require(structural_raytracing_call_shader)]
    public void callShader<CallableContext>(
        uint callableIndex,
        TraceProgramDescriptor<Schema> descriptor,
        inout CallableContext.CallableData data)
        where CallableContext.TraceContext == Schema.TraceContext
        // The callable-context witness is intentionally not the first hidden generic argument;
        // compiler-owned lowering discovers it from the conformance requirement.
        where CallableContext : ICallableContext
    {
        __target_switch
        {
        case metal:
            // The compiler consumes this call as structural callable dispatch before emission.
            break;
        default:
            CallShader(callableIndex, data);
        }
    }
}

} // namespace rt
