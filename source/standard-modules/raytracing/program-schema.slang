implementing raytracing;

namespace rt
{

/// Describes the closest-hit, any-hit, and intersection stages available as one logical hit group.
///
/// The group describes executable code and its shared context, not a physical SBT record. A host
/// may place the same group in any number of records and provide different `Context.Record` values.
public interface IHitGroup
{
    associatedtype Context : IHitContext;
    associatedtype ClosestHit : IClosestHitShader;
    associatedtype AnyHit : IAnyHitShader;
    associatedtype Intersection : IIntersectionStage;
    __constraint ClosestHit.Context == Context;
    __constraint AnyHit.Context == Context;
    __constraint Intersection.Context == Context;
}

/// Marks the hit-group section of a trace program schema.
///
/// The interface is sealed so the compiler only needs to decode the standard closed-list and
/// open-section forms.
[sealed]
public interface IHitGroupList
{}

/// Marks the miss-shader section of a trace program schema.
///
/// The interface is sealed so the compiler only needs to decode the standard section forms. A miss
/// SBT record binds one shader, so miss entries do not need a wrapper group type.
[sealed]
public interface IMissShaderList
{}

/// Marks the callable-shader section of a trace program schema.
///
/// The interface is sealed so the compiler only needs to decode the standard section forms. A
/// callable SBT record binds one shader, so callable entries do not need a wrapper group type.
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

/// Selects a hit-group section that is completed when separately compiled modules are linked.
///
/// Use `HitGroupList<A, B>` when the schema's module can enumerate every group. An open section
/// instead lets independently compiled modules opt their groups into one application-owned tag:
///
///     public interface IMaterialHitGroup : rt::IHitGroup {}
///     // In the schema: typealias HitGroups = rt::OpenHitGroups<IMaterialHitGroup>;
///     // In a separately compiled material module: struct GlassHit : IMaterialHitGroup { ... }
///
/// The finalized section includes concrete conformers such as GlassHit from the modules the host
/// links, without editing the shared schema to name each one. It does not discover unloaded code
/// or permit arbitrary runtime shader registration. The tag selects this program's groups rather
/// than every IHitGroup in the linked modules. The type is public because application schemas name
/// it in their associated-type definitions; closed lists remain sufficient for centrally listed
/// programs.
///
/// `Group` names optional entries known by the schema's module. The compiler additionally requires
/// `Tag` itself to be an interface so a concrete group cannot accidentally stand in for a tag.
/// TODO: implement linked-conformer collection in the schema-completion slice. This frontend slice
/// validates the tag and explicitly listed entries; `knownCount` counts only those listed entries.
public struct OpenHitGroups<Tag, each Group> : IHitGroupList
    where Tag : IHitGroup
    where expand each Group : IHitGroup
{
    public static const int knownCount = countof(Group);
}

/// Selects a miss-shader section that is completed when separately compiled modules are linked.
///
/// This is the miss-stage counterpart of `OpenHitGroups`: a shared schema can name an application
/// tag such as `IEnvironmentMiss : IMissShader`, while separately compiled modules provide its
/// concrete stages. Use `MissShaderList` when every stage can be named directly.
/// Listed shaders retain declaration order. Linked shaders conforming to `Tag` follow them in
/// qualified-type-name order within each payload partition. TODO: collect those conformers in the
/// schema-completion slice; `knownCount` currently describes only the explicitly listed stages.
public struct OpenMissShaders<Tag, each Shader> : IMissShaderList
    where Tag : IMissShader
    where expand each Shader : IMissShader
{
    public static const int knownCount = countof(Shader);
}

/// Selects a callable-shader section completed from conformances in the linked program.
///
/// This is the callable-stage counterpart of `OpenHitGroups`: independently compiled modules can
/// contribute stages through an application tag such as `IMaterialCallable : ICallableShader`.
/// Use `CallableShaderList` when the schema's module can name every callable directly.
/// The compiler additionally requires `Tag` itself to be an interface. Linked callables follow the
/// listed shaders in qualified-type-name order in the program-wide callable table. TODO: collect
/// those conformers in the schema-completion slice; `knownCount` counts only the listed stages.
public struct OpenCallableShaders<Tag, each Shader> : ICallableShaderList
    where Tag : ICallableShader
    where expand each Shader : ICallableShader
{
    public static const int knownCount = countof(Shader);
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
    associatedtype TraceContext : ITraceContext;
    associatedtype HitGroups : IHitGroupList;
    associatedtype MissShaders : IMissShaderList;
    associatedtype CallableShaders : ICallableShaderList;
}

} // namespace rt
