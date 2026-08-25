implementing raytracing;

namespace rt
{

public struct RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.AccelerationStructure == AccelerationStructure
    where ProgramLayout.TraceContext.Motion == NoMotion
{
    [ForceInline]
    public void trace(
        RayTraversalDesc desc,
        ProgramLayout.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
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

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.AccelerationStructure == AccelerationStructure
    where ProgramLayout.TraceContext.Motion == PrimitiveMotion
{
    [ForceInline]
    [require(metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        ProgramLayout.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.AccelerationStructure == AccelerationStructure
    where ProgramLayout.TraceContext.Motion == InstanceMotion
{
    [ForceInline]
    [require(metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        ProgramLayout.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.AccelerationStructure == AccelerationStructure
    where ProgramLayout.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    [ForceInline]
    [require(metallib_2_4)]
    public void trace(
        RayTraversalDesc desc,
        ProgramLayout.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.Motion == NoMotion
{
    [ForceInline]
    [require(metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
        where ProgramLayout.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.Motion == PrimitiveMotion
{
    [ForceInline]
    [require(metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
        where ProgramLayout.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.Motion == InstanceMotion
{
    [ForceInline]
    [require(metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
        where ProgramLayout.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.Motion == PrimitiveAndInstanceMotion
{
    [ForceInline]
    [require(metal)]
    public void trace<let maxLevelCount : int>(
        RayTraversalDesc desc,
        MultiLevelAccelerationStructure<maxLevelCount> accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
        where ProgramLayout.TraceContext.AccelerationStructure ==
            MultiLevelAccelerationStructure<maxLevelCount>
    {
        // The compiler consumes this call as a structural trace operation before emission.
    }
}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{
    [ForceInline]
    [require(structural_raytracing_call_shader)]
    public void callShader<CallableContext>(
        uint callableIndex,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout CallableContext.CallableData data)
        where CallableContext : ICallableGroupContext
        where CallableContext.TraceContext == ProgramLayout.TraceContext
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
