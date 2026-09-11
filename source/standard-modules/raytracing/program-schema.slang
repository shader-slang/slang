implementing raytracing;

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

public typealias HitGroupSlot<let indexValue : int> = ShaderGroupSlot<indexValue>;
public typealias MissSlot<let indexValue : int> = ShaderGroupSlot<indexValue>;
public typealias CallableSlot<let indexValue : int> = ShaderGroupSlot<indexValue>;

/// Describes the closest-hit, any-hit, and intersection stages associated with one hit record.
///
/// All three stages must use the group's `Context`, so the type system fixes one payload,
/// primitive, record, and custom-attribute contract for the complete hit group.
public interface IHitGroup
{
    associatedtype Slot : IShaderGroupSlot;
    associatedtype Context : IHitContext;
    associatedtype ClosestHit : IClosestHitShader;
    associatedtype AnyHit : IAnyHitShader;
    associatedtype Intersection : IIntersectionStage;
    __constraint ClosestHit.Context == Context;
    __constraint AnyHit.Context == Context;
    __constraint Intersection.Context == Context;
}

/// Describes the miss stage associated with one miss record.
///
/// The explicit constraint makes the group's payload and record context identical to the context
/// declared by its miss-stage implementation.
public interface IMissGroup
{
    associatedtype Slot : IShaderGroupSlot;
    associatedtype Context : IMissGroupContext;
    associatedtype Miss : IMissShader;
    __constraint Miss.Context == Context;
}

/// Describes the callable stage associated with one callable record.
///
/// The explicit constraint makes the group's callable-data and record context identical to the
/// context declared by its callable-stage implementation.
public interface ICallableGroup
{
    associatedtype Slot : IShaderGroupSlot;
    associatedtype Context : ICallableGroupContext;
    associatedtype Callable : ICallableShader;
    __constraint Callable.Context == Context;
}

[sealed]
public interface IHitGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    static const int count;
}

[sealed]
public interface IMissGroupList<TraceContext>
    where TraceContext : ITraceContext
{
    static const int count;
}

[sealed]
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

public interface ITraceProgramSchema
{
    associatedtype TraceContext : ITraceContext;
    associatedtype MissGroups : IMissGroupList<TraceContext>;
    associatedtype HitGroups : IHitGroupList<TraceContext>;
    associatedtype CallableGroups : ICallableGroupList<TraceContext>;
}

} // namespace rt
