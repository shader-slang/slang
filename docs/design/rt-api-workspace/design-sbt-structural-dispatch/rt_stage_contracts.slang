implementing rt_pipeline;

namespace rt
{

// User stage structs implement invoke(). The compiler maps the specialized
// conformance directly to the corresponding native ray-tracing stage.
public interface IClosestHitShader<Context>
    where Context : IHitContext
{
    void invoke(ClosestHitInput<Context> input);
}

public interface IAnyHitShader<Context>
    where Context : IHitContext
{
    void invoke(AnyHitInput<Context> input);
}

// This marker lets a hit group name either an intersection stage or the
// built-in NoIntersection placeholder without inventing fake attributes.
public interface IIntersectionStage<Context>
    where Context : IHitContext
{}

public interface IIntersectionShader<Context> : IIntersectionStage<Context>
    where Context : IHitContext
    where Context.Primitive : ICustomIntersectionPrimitive
{
    void invoke(IntersectionInput<Context> input);
}

public interface IMissShader<Context>
    where Context : IMissGroupContext
{
    void invoke(MissInput<Context> input);
}

public interface ICallableShader<Context>
    where Context : ICallableGroupContext
{
    void invoke(CallableInput<Context> input);
}

// The compiler recognizes these placeholders as absent functions. They do not
// consume entries in the native SBT or Metal function tables.
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
