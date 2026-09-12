implementing raytracing;

namespace rt
{

/// Describes the closest-hit, any-hit, and intersection stages available as one native hit group.
///
/// The group describes executable code and its shared context, not a physical SBT record. A host
/// may place the same group in any number of records and provide different `Context.Record` values.
public interface IHitGroup
{
    associatedtype Context;
    __constraint Context : IHitContext;
    associatedtype ClosestHit;
    __constraint ClosestHit : IClosestHitShader;
    associatedtype AnyHit;
    __constraint AnyHit : IAnyHitShader;
    associatedtype Intersection;
    __constraint Intersection : IIntersectionStage;
    __constraint ClosestHit.Context == Context;
    __constraint AnyHit.Context == Context;
    __constraint Intersection.Context == Context;
}

/// Marks the closed set of hit groups available to a trace program schema.
///
/// The interface is sealed so the compiler only needs to decode the standard list forms.
[sealed]
public interface IHitGroupList
{}

/// Marks the closed set of miss shaders available to a trace program schema.
///
/// A miss SBT record binds one shader, so miss entries do not need a wrapper group type.
[sealed]
public interface IMissShaderList
{}

/// Marks the closed set of callable shaders available to a trace program schema.
///
/// A callable SBT record binds one shader, so callable entries do not need a wrapper group type.
[sealed]
public interface ICallableShaderList
{}

/// Lists the hit-group implementations available to a trace program schema.
///
/// Declaration order determines the dense function index within each payload partition. It does
/// not determine where a host places records in the runtime SBT.
public struct HitGroupList<each Group> : IHitGroupList
    where expand each Group : IHitGroup
{
    public static const int count = countof(Group);
}

/// Lists the miss-stage implementations available to a trace program schema.
///
/// Declaration order determines the dense function index within each payload partition.
public struct MissShaderList<each Shader> : IMissShaderList
    where expand each Shader : IMissShader
{
    public static const int count = countof(Shader);
}

/// Lists the callable-stage implementations available to a trace program schema.
///
/// Declaration order determines the program-wide dense function index.
public struct CallableShaderList<each Shader> : ICallableShaderList
    where expand each Shader : ICallableShader
{
    public static const int count = countof(Shader);
}

/// Selects an empty hit-group section.
public struct NoHitGroups : IHitGroupList
{}

/// Selects an empty miss-shader section.
public struct NoMissShaders : IMissShaderList
{}

/// Selects an empty callable-shader section.
public struct NoCallableShaders : ICallableShaderList
{}

/// Describes the finite set of ray-tracing code available to one linked program.
///
/// The schema contains no physical SBT positions. Reflection maps these entry types to target
/// identifiers or link-specific function indices that the host writes into runtime records.
public interface ITraceProgramSchema
{
    associatedtype TraceContext;
    __constraint TraceContext : ITraceContext;
    associatedtype HitGroups;
    __constraint HitGroups : IHitGroupList;
    associatedtype MissShaders;
    __constraint MissShaders : IMissShaderList;
    associatedtype CallableShaders;
    __constraint CallableShaders : ICallableShaderList;
}

} // namespace rt
