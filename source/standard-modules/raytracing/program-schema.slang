implementing raytracing;

namespace rt
{

/// Describes the closest-hit, any-hit, and intersection stages available as one logical hit group.
///
/// The group describes executable code and its shared context, not a physical SBT record. A host
/// may place the same group in any number of records and provide different `Context.Record` values.
public interface IHitGroup
{
    /// Defines the payload, primitive, and shader-record types shared by the hit stages.
    ///
    /// @par Constraints
    /// Must conform to `IHitContext`. Each of `ClosestHit.Context`, `AnyHit.Context`, and
    /// `Intersection.Context` must equal this type.
    associatedtype Context : IHitContext;

    /// Selects the closest-hit implementation, or `NoClosestHit<Context>` to omit that stage.
    ///
    /// @par Constraints
    /// Must conform to `IClosestHitShader`, with `ClosestHit.Context == Context`.
    associatedtype ClosestHit : IClosestHitShader;

    /// Selects the any-hit implementation, or `NoAnyHit<Context>` to omit that stage.
    ///
    /// @par Constraints
    /// Must conform to `IAnyHitShader`, with `AnyHit.Context == Context`.
    associatedtype AnyHit : IAnyHitShader;

    /// Selects a procedural intersection implementation or the hardware-intersection placeholder.
    ///
    /// @par Constraints
    /// Must conform to `IIntersectionStage`, with `Intersection.Context == Context`. Procedural
    /// implementations conform to `IIntersectionShader`; hardware-intersected primitives use
    /// `NoIntersection<Context>`.
    associatedtype Intersection : IIntersectionStage;
    __constraint ClosestHit.Context == Context;
    __constraint AnyHit.Context == Context;
    __constraint Intersection.Context == Context;
}

/// Marks the hit-group section of a trace program schema.
///
/// The interface is sealed so the compiler only needs to decode `HitGroupList` and `NoHitGroups`.
[sealed]
public interface IHitGroupList
{}

/// Marks the miss-shader section of a trace program schema.
///
/// The interface is sealed so the compiler only needs to decode `MissShaderList` and
/// `NoMissShaders`. A miss SBT record binds one shader, so miss entries do not need a wrapper group
/// type.
[sealed]
public interface IMissShaderList
{}

/// Marks the callable-shader section of a trace program schema.
///
/// The interface is sealed so the compiler only needs to decode `CallableShaderList` and
/// `NoCallableShaders`. A callable SBT record binds one shader, so callable entries do not need a
/// wrapper group type.
[sealed]
public interface ICallableShaderList
{}

/// Lists the hit-group implementations available to a trace program schema.
///
/// The planned target layout assigns dense function indices in declaration order across the
/// schema's entire hit section, including groups with different payload types. These indices do
/// not determine where a host places records in the runtime SBT.
/// @tparam Group The type pack of hit-group implementations in declaration order.
///
/// @par Constraints
/// Every type in `Group` must conform to `IHitGroup`. This list conforms to `IHitGroupList`.
public struct HitGroupList<each Group> : IHitGroupList
    where expand each Group : IHitGroup
{
    /// Gives the number of hit-group types in `Group`.
    ///
    /// @par Constraints
    /// A compile-time constant equal to `countof(Group)`, including zero for an empty pack.
    public static const int count = countof(Group);
}

/// Lists the miss-stage implementations available to a trace program schema.
///
/// The planned target layout assigns dense function indices in declaration order across the
/// schema's entire miss section, including shaders with different payload types. Runtime miss
/// record indices remain host-selected positions in one shared miss section.
/// @tparam Shader The type pack of miss-stage implementations in declaration order.
///
/// @par Constraints
/// Every type in `Shader` must conform to `IMissShader`. This list conforms to `IMissShaderList`.
public struct MissShaderList<each Shader> : IMissShaderList
    where expand each Shader : IMissShader
{
    /// Gives the number of miss-stage types in `Shader`.
    ///
    /// @par Constraints
    /// A compile-time constant equal to `countof(Shader)`, including zero for an empty pack.
    public static const int count = countof(Shader);
}

/// Lists the callable-stage implementations available to a trace program schema.
///
/// Declaration order determines the program-wide dense function index.
/// @tparam Shader The type pack of callable-stage implementations in declaration order.
///
/// @par Constraints
/// Every type in `Shader` must conform to `ICallableShader`. This list conforms to
/// `ICallableShaderList`.
public struct CallableShaderList<each Shader> : ICallableShaderList
    where expand each Shader : ICallableShader
{
    /// Gives the number of callable-stage types in `Shader`.
    ///
    /// @par Constraints
    /// A compile-time constant equal to `countof(Shader)`, including zero for an empty pack.
    public static const int count = countof(Shader);
}

/// Selects an empty hit-group section.
///
/// @par Constraints
/// Conforms to `IHitGroupList`, with no hit-group entries.
public struct NoHitGroups : IHitGroupList
{}

/// Selects an empty miss-shader section.
///
/// @par Constraints
/// Conforms to `IMissShaderList`, with no miss-stage entries.
public struct NoMissShaders : IMissShaderList
{}

/// Selects an empty callable-shader section.
///
/// @par Constraints
/// Conforms to `ICallableShaderList`, with no callable-stage entries.
public struct NoCallableShaders : ICallableShaderList
{}

/// Describes the finite set of ray-tracing code available to one linked program.
///
/// The schema contains no physical SBT positions. Reflection maps these entry types to target
/// identifiers or link-specific function indices that the host writes into runtime records.
/// Each section names an explicit list or the corresponding empty-section type.
public interface ITraceProgramSchema
{
    /// Defines the acceleration structure and motion policy used to trace this program.
    ///
    /// @par Constraints
    /// Must conform to `ITraceContext`.
    associatedtype TraceContext : ITraceContext;

    /// Selects the finite list of hit groups available to this program.
    ///
    /// @par Constraints
    /// Must conform to the sealed `IHitGroupList`: use `HitGroupList<...>` or `NoHitGroups`.
    associatedtype HitGroups : IHitGroupList;

    /// Selects the finite list of miss stages available to this program.
    ///
    /// @par Constraints
    /// Must conform to the sealed `IMissShaderList`: use `MissShaderList<...>` or `NoMissShaders`.
    associatedtype MissShaders : IMissShaderList;

    /// Selects the finite list of callable stages available to this program.
    ///
    /// @par Constraints
    /// Must conform to the sealed `ICallableShaderList`: use `CallableShaderList<...>` or
    /// `NoCallableShaders`.
    associatedtype CallableShaders : ICallableShaderList;
}

} // namespace rt
