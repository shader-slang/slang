implementing rt_pipeline;

namespace rt
{

// Stage contracts. These are ordinary functor structs in this sketch, but they
// correspond to the pipeline stages that would become native entry points on
// D3D/Vulkan and generated helper functions on Metal.

public interface IClosestHitShader<TContext : IHitContext>
    : IFunc<void, ClosestHitInput<TContext>>
{
}

public interface IAnyHitShader<TContext : IHitContext>
    : IFunc<void, AnyHitInput<TContext>>
{
}

public interface IIntersectionShader<TContext : IHitContext, TAttributes>
    : IFunc<IntersectionReturn<TContext, TAttributes>, IntersectionInput<TContext>>
{
}

public interface IMissShader<TTraceContext : ITraceContext>
    : IFunc<void, MissInput<TTraceContext>>
{
}

public struct NoAnyHit<TContext : IHitContext> : IAnyHitShader<TContext>
{
    public void operator()(AnyHitInput<TContext> input)
    {
    }
}

public struct NoIntersection<TContext : IHitContext> : IIntersectionShader<TContext, NoAttributes>
{
    public typealias Attributes = NoAttributes;

    public IntersectionReturn<TContext, Attributes> operator()(IntersectionInput<TContext> input)
    {
        IntersectionReturn<TContext, Attributes> result;
        result.acceptIntersection = false;
        result.continueSearch = true;
        result.distance = 0.0;
        return result;
    }
}

} // namespace rt
