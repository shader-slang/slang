implementing raytracing;

namespace rt
{

public struct RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{}

public extension<ProgramLayout> RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
    where ProgramLayout.TraceContext.AccelerationStructure == AccelerationStructure
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
