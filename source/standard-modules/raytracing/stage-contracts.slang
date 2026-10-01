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
    associatedtype Context : IHitContext;
    void invoke(in ClosestHitInput<Context> input);
}

/// Represents the source implementation of an any-hit stage.
///
/// `Context` fixes the candidate's payload, primitive, and shader-record types. The compiler turns
/// `invoke` into the target's any-hit behavior and preserves `ignoreHit` and candidate-termination
/// control flow.
[require(structural_raytracing_anyhit)]
public interface IAnyHitShader
{
    associatedtype Context : IHitContext;
    void invoke(in AnyHitInput<Context> input);
}

/// Represents either an intersection shader or the canonical `NoIntersection` placeholder.
///
/// `IHitGroup.Intersection` must accept both a procedural shader and `NoIntersection<Context>`
/// for hardware-intersected triangles or curves, while checking that either choice uses the
/// group's context. For example, a triangle group sets:
///
///     typealias Intersection = NoIntersection<Context>;
///
/// A procedural group instead names a struct implementing `IIntersectionShader`. Requiring that
/// interface for both choices would force the hardware placeholder to implement `invoke` and
/// satisfy its custom-primitive constraint, even though hardware intersection has neither.
/// Keeping only `Context` here represents the shared requirement without inventing an executable
/// intersection shader for a builtin primitive. Sealing this interface prevents user-defined
/// placeholders from bypassing the procedural shader contract.
[sealed]
public interface IIntersectionStage
{
    associatedtype Context : IHitContext;
}

/// Represents the source implementation of a procedural-primitive intersection stage.
///
/// `IIntersectionStage` supplies the same `Context` requirement used by `IHitGroup.Intersection`
/// and the hardware-intersection placeholder. This executable branch additionally requires a
/// custom primitive and an `invoke` implementation: `Context.Primitive` supplies the attributes
/// reported by `input.reportHit`. Adapter synthesis must turn `invoke` into the target's native
/// intersection mechanism. Closest-hit and any-hit need no corresponding split because their
/// optional no-op implementations satisfy the same stage contract as an ordinary shader.
[require(structural_raytracing_intersection)]
public interface IIntersectionShader : IIntersectionStage
{
    __constraint Context.Primitive : ICustomIntersectionPrimitive;
    void invoke(in IntersectionInput<Context> input);
}

/// Represents the source implementation of a miss stage.
///
/// `Context` fixes the trace payload and miss-record types. The compiler turns `invoke` into a
/// native miss entry point or the corresponding structural dispatch function.
[require(structural_raytracing_miss)]
public interface IMissShader
{
    associatedtype Context : IPayloadContext;
    void invoke(in MissInput<Context> input);
}

/// Represents the source implementation of a callable stage.
///
/// `Context` fixes the callable-data and callable-record types. The compiler turns `invoke` into a
/// native callable entry point or the corresponding structural dispatch function.
[require(structural_raytracing_callable)]
public interface ICallableShader
{
    associatedtype Context : ICallableContext;
    void invoke(in CallableInput<Context> input);
}

/// Selects no closest-hit stage for a hit group.
///
/// Its empty `invoke` satisfies the source contract, while structural lowering omits the native
/// stage when the target does not need a placeholder function.
public struct NoClosestHit<C> : IClosestHitShader
    where C : IHitContext
{
    public typealias Context = C;
    public void invoke(in ClosestHitInput<C> input) {}
}

/// Selects no any-hit stage for a hit group.
///
/// Its empty `invoke` satisfies the source contract; structural lowering recognizes this exact
/// standard-module type and does not emit user any-hit behavior.
public struct NoAnyHit<C> : IAnyHitShader
    where C : IHitContext
{
    public typealias Context = C;
    public void invoke(in AnyHitInput<C> input) {}
}

/// Selects hardware intersection instead of a user intersection stage.
///
/// Structural lowering recognizes this exact standard-module type and omits a procedural
/// intersection entry point. Unlike a real `IIntersectionShader`, the placeholder has no `invoke`.
public struct NoIntersection<C> : IIntersectionStage
    where C : IHitContext
    where C.Primitive : IBuiltinIntersectionPrimitive
{
    public typealias Context = C;
}

} // namespace rt
