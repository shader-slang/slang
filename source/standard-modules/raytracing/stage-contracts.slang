implementing raytracing;

namespace rt
{

[require(structural_raytracing_closesthit)]
public interface IClosestHitShader<Context>
    where Context : IHitContext
{
    void invoke(ClosestHitInput<Context> input);
}

[require(structural_raytracing_anyhit)]
public interface IAnyHitShader<Context>
    where Context : IHitContext
{
    void invoke(AnyHitInput<Context> input);
}

// This compiler-owned marker admits either IIntersectionShader or the built-in NoIntersection
// placeholder without exposing a third extension point to user code.
internal interface IIntersectionStage<Context>
    where Context : IHitContext
{}

[require(structural_raytracing_intersection)]
public interface IIntersectionShader<Context> : IIntersectionStage<Context>
    where Context : IHitContext
{
    __constraint Context.Primitive : ICustomIntersectionPrimitive;
    void invoke(IntersectionInput<Context> input);
}

[require(structural_raytracing_miss)]
public interface IMissShader<Context>
    where Context : IMissGroupContext
{
    void invoke(MissInput<Context> input);
}

[require(structural_raytracing_callable)]
public interface ICallableShader<Context>
    where Context : ICallableGroupContext
{
    void invoke(CallableInput<Context> input);
}

public struct NoClosestHit<Context> : IClosestHitShader<Context>
    where Context : IHitContext
{
    public void invoke(ClosestHitInput<Context> input) {}
}

public struct NoAnyHit<Context> : IAnyHitShader<Context>
    where Context : IHitContext
{
    public void invoke(AnyHitInput<Context> input) {}
}

public struct NoIntersection<Context> : IIntersectionStage<Context>
    where Context : IHitContext
{}

} // namespace rt
