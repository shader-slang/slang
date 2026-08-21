// Structural shader-binding-table layout for the rt_pipeline proof of concept.

implementing rt_pipeline;

namespace rt
{

public interface IShaderGroupSlot
{
    static const int index;
}

public struct ShaderGroupSlot<let indexValue : int> : IShaderGroupSlot
{
    public static const int index = indexValue;
}

// Section-specific names improve readability while sharing one slot contract.
public typealias HitGroupSlot<let indexValue : int> = ShaderGroupSlot<indexValue>;
public typealias MissSlot<let indexValue : int> = ShaderGroupSlot<indexValue>;
public typealias CallableSlot<let indexValue : int> = ShaderGroupSlot<indexValue>;

public uint getHitGroupSlot(
    RayTraversalDesc desc,
    uint geometryContribution,
    uint instanceContribution)
{
    return instanceContribution + geometryContribution * desc.sbtStride + desc.sbtOffset;
}

public interface IHitGroup
{
    associatedtype Slot : IShaderGroupSlot;
    associatedtype Context : IHitContext;
    associatedtype ClosestHit : IClosestHitShader<Context>;
    associatedtype AnyHit : IAnyHitShader<Context>;
    associatedtype Intersection : IIntersectionStage<Context>;
}

public interface IMissGroup
{
    associatedtype Slot : IShaderGroupSlot;
    associatedtype Context : IMissGroupContext;
    associatedtype Miss : IMissShader<Context>;
}

public interface ICallableGroup
{
    associatedtype Slot : IShaderGroupSlot;
    associatedtype Context : ICallableGroupContext;
    associatedtype Callable : ICallableShader<Context>;
}

public interface IHitGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    static const int count;
}

public interface IMissGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    static const int count;
}

public interface ICallableGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    static const int count;
}

public struct HitGroupList<TraceContext, each Group> : IHitGroupList<TraceContext>
    where TraceContext : ITraceContext
    where Group : IHitGroup
    where expand each Group.Context.TraceContext == TraceContext
{
    public static const int count = countof(Group);
}

public struct MissGroupList<TraceContext, each Group> : IMissGroupList<TraceContext>
    where TraceContext : ITraceContext
    where Group : IMissGroup
    where expand each Group.Context.TraceContext == TraceContext
{
    public static const int count = countof(Group);
}

public struct CallableGroupList<TraceContext, each Group> : ICallableGroupList<TraceContext>
    where TraceContext : ITraceContext
    where Group : ICallableGroup
    where expand each Group.Context.TraceContext == TraceContext
{
    public static const int count = countof(Group);
}

public struct NoHitGroups<TraceContext> : IHitGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    public static const int count = 0;
}

public struct NoMissGroups<TraceContext> : IMissGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    public static const int count = 0;
}

public struct NoCallableGroups<TraceContext> : ICallableGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    public static const int count = 0;
}

public interface ITraceProgramLayout
{
    associatedtype TraceContext : ITraceContext;
    associatedtype MissGroups : IMissGroupList<TraceContext>;
    associatedtype HitGroups : IHitGroupList<TraceContext>;
    associatedtype CallableGroups : ICallableGroupList<TraceContext>;
}

// A ParameterBlock-like shader-visible resource. Its physical fields are
// selected after ProgramLayout specialization and differ by target.
public struct TraceProgramDescriptor<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{}

public struct RayTracer<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{
    public void trace(
        RayTraversalDesc desc,
        ProgramLayout.TraceContext.AccelerationStructure accelerationStructure,
        TraceProgramDescriptor<ProgramLayout> descriptor,
        inout ProgramLayout.TraceContext.Payload payload)
    {
        __rtTraceRayWithProgram<ProgramLayout>(
            desc,
            accelerationStructure,
            descriptor,
            payload);
    }
}

// Compiler-known structural trace marker. D3D/Vulkan reuse the existing
// TraceRay path; Metal uses ProgramLayout to synthesize traversal and dispatch.
void __rtTraceRayWithProgram<ProgramLayout>(
    RayTraversalDesc desc,
    ProgramLayout.TraceContext.AccelerationStructure accelerationStructure,
    TraceProgramDescriptor<ProgramLayout> descriptor,
    inout ProgramLayout.TraceContext.Payload payload)
    where ProgramLayout : ITraceProgramLayout;

} // namespace rt
