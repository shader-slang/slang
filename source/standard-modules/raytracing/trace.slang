implementing raytracing;

namespace rt
{

public struct RayTracer<Schema>
    where Schema : ITraceProgramSchema
{}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == NoMotion
{
    [ForceInline]
    [require(structural_raytracing_trace)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
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

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    [ForceInline]
    [require(structural_raytracing_trace, metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == InstanceMotion
{
    [ForceInline]
    [require(structural_raytracing_trace, metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == AccelerationStructure
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    [ForceInline]
    [require(structural_raytracing_trace, metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        Schema.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == NoMotion
{
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveMotion
{
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == InstanceMotion
{
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
    where Schema.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    [ForceInline]
    [require(structural_raytracing_trace, metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<Schema> descriptor,
        inout Schema.TraceContext.Payload payload)
        where Schema.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<Schema> RayTracer<Schema>
    where Schema : ITraceProgramSchema
{
    [ForceInline]
    [require(structural_raytracing_call_shader)]
    public void callShader<CallableContext>(
        uint callableIndex,
        TraceProgramDescriptor<Schema> descriptor,
        inout CallableContext.CallableData data)
        where CallableContext : ICallableGroupContext
        where CallableContext.TraceContext == Schema.TraceContext
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
