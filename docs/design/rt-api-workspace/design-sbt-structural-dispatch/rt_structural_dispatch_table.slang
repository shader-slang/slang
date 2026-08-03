// Draft Slang-side structural ray dispatch table.
//
// This fragment adds structural dispatch cases to the rt_pipeline module. The
// cases can lower in two ways:
//
// - Metal: generate a post-intersect closest-hit/miss dispatch switch.
// - D3D/Vulkan: generate or reflect native miss/hit-group entry points.

implementing rt_pipeline;

namespace rt
{

public interface IHitGroupSlot
{
    static const int index;
}

public struct HitGroupSlot<let indexValue : int> : IHitGroupSlot
{
    public static const int index = indexValue;
}

public interface IMissSlot
{
    static const int index;
}

public struct MissSlot<let indexValue : int> : IMissSlot
{
    public static const int index = indexValue;
}

public struct RayDispatch
{
    // D3D: RayContributionToHitGroupIndex.
    // Vulkan: sbtRecordOffset.
    // Metal IFB: intersector::set_base_id(...).
    public uint sbtOffset;

    // D3D: MultiplierForGeometryContributionToHitGroupIndex.
    // Vulkan: sbtRecordStride.
    // Metal IFB: intersector::set_geometry_multiplier(...).
    public uint sbtStride;

    public uint missIndex;
}

public uint getHitGroupSlot(
    RayDispatch dispatch,
    uint geometryContribution,
    uint instanceContribution)
{
    return instanceContribution + geometryContribution * dispatch.sbtStride +
        dispatch.sbtOffset;
}

public interface IHitGroup<TTraceContext : ITraceContext>
{
}

public interface IMissGroup<TTraceContext : ITraceContext>
{
}

public struct HitGroup<
    TTraceContext : ITraceContext,
    TSlot : IHitGroupSlot,
    TContext : IHitContextFor<TTraceContext>,
    TClosestHit : IClosestHitShader<TContext>,
    TAnyHit : IAnyHitShader<TContext>,
    TIntersectionAttributes,
    TIntersection : IIntersectionShader<TContext, TIntersectionAttributes>>
    : IHitGroup<TTraceContext>
{
    public typealias Slot = TSlot;
    public typealias Context = TContext;
    public typealias ClosestHit = TClosestHit;
    public typealias AnyHit = TAnyHit;
    public typealias IntersectionAttributes = TIntersectionAttributes;
    public typealias Intersection = TIntersection;
}

public struct MissGroup<
    TTraceContext : ITraceContext,
    TSlot : IMissSlot,
    TMiss : IMissShader<TTraceContext>>
    : IMissGroup<TTraceContext>
{
    public typealias Slot = TSlot;
    public typealias Miss = TMiss;
}

public interface IHitGroupList<TTraceContext : ITraceContext>
{
    static const int count;
}

public interface IMissGroupList<TTraceContext : ITraceContext>
{
    static const int count;
}

public struct HitGroupList<
    TTraceContext : ITraceContext,
    each TGroup : IHitGroup<TTraceContext>>
    : IHitGroupList<TTraceContext>
{
    public static const int count = countof(TGroup);
}

public struct MissGroupList<
    TTraceContext : ITraceContext,
    each TGroup : IMissGroup<TTraceContext>>
    : IMissGroupList<TTraceContext>
{
    public static const int count = countof(TGroup);
}

public interface TraceProgram
{
    associatedtype TraceContext : ITraceContext;
    associatedtype MissGroups : IMissGroupList<TraceContext>;
    associatedtype HitGroups : IHitGroupList<TraceContext>;
}

uint __rtGetInstanceContribution<TProgram : TraceProgram>(
    IntersectionResult<
        TProgram.TraceContext.ASKind,
        TProgram.TraceContext.dataTags,
        TProgram.TraceContext.Motion,
        TProgram.TraceContext.maxLevels> intersectionResult);

uint __rtGetGeometryContribution<TProgram : TraceProgram>(
    IntersectionResult<
        TProgram.TraceContext.ASKind,
        TProgram.TraceContext.dataTags,
        TProgram.TraceContext.Motion,
        TProgram.TraceContext.maxLevels> intersectionResult);

public struct RayTracer<TProgram : TraceProgram>
{
    public void trace(
        RayTraversalDesc desc,
        AccelerationStructure<TProgram.TraceContext.ASKind, TProgram.TraceContext.Motion> scene,
        RayDispatch dispatch,
        inout TProgram.TraceContext.Payload payload)
    {
        __rtTraceRayWithProgram<TProgram>(desc, scene, dispatch, payload);
    }
}

void __rtTraceRayWithProgram<TProgram : TraceProgram>(
    RayTraversalDesc desc,
    AccelerationStructure<TProgram.TraceContext.ASKind, TProgram.TraceContext.Motion> scene,
    RayDispatch dispatch,
    inout TProgram.TraceContext.Payload payload);

} // namespace rt





// ClostHit -> TRIANGLE/PROC
struct TriClosetHit : IClosetHit<Context>
{
    ///
}
