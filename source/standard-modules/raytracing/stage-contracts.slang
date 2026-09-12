implementing raytracing;

namespace rt
{

/// Represents the source implementation of a closest-hit stage.
///
/// `Context` fixes the payload, primitive, and shader-record types exposed through the stage
/// input. The compiler turns `invoke` into a native closest-hit entry point; shader code cannot
/// call it as an ordinary method.
[require(structural_raytracing_closesthit)]
public interface IClosestHitShader
{
    associatedtype Context;
    __constraint Context : IHitContext;
    void invoke(ClosestHitInput<Context> input);
}

/// Represents the source implementation of an any-hit stage.
///
/// `Context` fixes the candidate's payload, primitive, and shader-record types. The compiler turns
/// `invoke` into the target's any-hit behavior and preserves `ignoreHit` and candidate-termination
/// control flow.
[require(structural_raytracing_anyhit)]
public interface IAnyHitShader
{
    associatedtype Context;
    __constraint Context : IHitContext;
    void invoke(AnyHitInput<Context> input);
}

/// Represents either an intersection shader or the canonical `NoIntersection` placeholder.
///
/// This public name is required by the associated-type contract on `IHitGroup`. The sealed
/// conformance set keeps it compiler-owned: user code implements `IIntersectionShader` instead of
/// implementing this common interface directly.
[sealed]
public interface IIntersectionStage
{
    associatedtype Context;
    __constraint Context : IHitContext;
}

/// Represents the source implementation of a procedural-primitive intersection stage.
///
/// `Context.Primitive` supplies the custom attributes reported by `input.reportHit`. The compiler
/// lowers `invoke` to the native intersection mechanism for the selected target.
[require(structural_raytracing_intersection)]
public interface IIntersectionShader : IIntersectionStage
{
    __constraint Context.Primitive : ICustomIntersectionPrimitive;
    void invoke(IntersectionInput<Context> input);
}

/// Represents the source implementation of a miss stage.
///
/// `Context` fixes the trace payload and miss-record types. The compiler turns `invoke` into a
/// native miss entry point or the corresponding structural dispatch function.
[require(structural_raytracing_miss)]
public interface IMissShader
{
    associatedtype Context;
    __constraint Context : IPayloadContext;
    void invoke(MissInput<Context> input);
}

/// Represents the source implementation of a callable stage.
///
/// `Context` fixes the callable-data and callable-record types. The compiler turns `invoke` into a
/// native callable entry point or the corresponding structural dispatch function.
[require(structural_raytracing_callable)]
public interface ICallableShader
{
    associatedtype Context;
    __constraint Context : ICallableContext;
    void invoke(CallableInput<Context> input);
}

/// Selects no closest-hit stage for a hit group.
///
/// Its empty `invoke` satisfies the source contract, while structural lowering omits the native
/// stage when the target does not need a placeholder function.
public struct NoClosestHit<C> : IClosestHitShader
    where C : IHitContext
{
    public typealias Context = C;
    public void invoke(ClosestHitInput<C> input) {}
}

/// Selects no any-hit stage for a hit group.
///
/// Its empty `invoke` satisfies the source contract; structural lowering recognizes this exact
/// standard-module type and does not emit user any-hit behavior.
public struct NoAnyHit<C> : IAnyHitShader
    where C : IHitContext
{
    public typealias Context = C;
    public void invoke(AnyHitInput<C> input) {}
}

/// Selects hardware intersection instead of a user intersection stage.
///
/// Structural lowering recognizes this exact standard-module type and omits a procedural
/// intersection entry point. Unlike a real `IIntersectionShader`, the placeholder has no `invoke`.
public struct NoIntersection<C> : IIntersectionStage
    where C : IHitContext
{
    public typealias Context = C;
}

} // namespace rt
