implementing raytracing;

namespace rt
{

/// Represents the source implementation of a closest-hit stage.
///
/// `Context` fixes the payload, primitive, and shader-record types exposed through the stage
/// input. The compiler turns `invoke` into a native closest-hit entry point; shader code cannot
/// call it as an ordinary method.
///
/// @par Constraints
/// Requires the `structural_raytracing_closesthit` capability.
[require(structural_raytracing_closesthit)]
public interface IClosestHitShader
{
    /// Defines the payload, primitive, and shader-record types available to this stage.
    ///
    /// @par Constraints
    /// Must conform to `IHitContext`. A hit group using this shader requires this type to equal
    /// the group's `Context`.
    associatedtype Context : IHitContext;

    /// Implements the closest-hit stage selected for the committed hit.
    /// @param[in] input The committed-hit state described by `Context`.
    void invoke(in ClosestHitInput<Context> input);
}

/// Represents the source implementation of an any-hit stage.
///
/// `Context` fixes the candidate's payload, primitive, and shader-record types. The compiler turns
/// `invoke` into the target's any-hit behavior and preserves `ignoreHit` and candidate-termination
/// control flow.
///
/// @par Constraints
/// Requires the `structural_raytracing_anyhit` capability.
[require(structural_raytracing_anyhit)]
public interface IAnyHitShader
{
    /// Defines the payload, primitive, and shader-record types available to this stage.
    ///
    /// @par Constraints
    /// Must conform to `IHitContext`. A hit group using this shader requires this type to equal
    /// the group's `Context`.
    associatedtype Context : IHitContext;

    /// Implements the any-hit stage for an intersection candidate.
    /// @param[in] input The candidate-hit state described by `Context`.
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
    /// Defines the shared context for procedural or hardware intersection.
    ///
    /// @par Constraints
    /// Must conform to `IHitContext`. A hit group using this stage requires this type to equal
    /// the group's `Context`.
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
///
/// @par Constraints
/// Inherits `Context : IHitContext` from `IIntersectionStage` and further requires
/// `Context.Primitive : ICustomIntersectionPrimitive`. Requires the
/// `structural_raytracing_intersection` capability.
[require(structural_raytracing_intersection)]
public interface IIntersectionShader : IIntersectionStage
{
    __constraint Context.Primitive : ICustomIntersectionPrimitive;

    /// Tests the procedural primitive and reports its intersections through `input.reportHit`.
    /// @param[in] input The ray and primitive state described by `Context` before reporting a hit.
    void invoke(in IntersectionInput<Context> input);
}

/// Represents the source implementation of a miss stage.
///
/// `Context` fixes the trace payload and miss-record types. The compiler turns `invoke` into a
/// native miss entry point or the corresponding structural dispatch function.
///
/// @par Constraints
/// Requires the `structural_raytracing_miss` capability.
[require(structural_raytracing_miss)]
public interface IMissShader
{
    /// Defines the trace payload and shader-record types available to this stage.
    ///
    /// @par Constraints
    /// Must conform to `IPayloadContext`.
    associatedtype Context : IPayloadContext;

    /// Implements the miss stage for a trace that finds no accepted hit.
    /// @param[in] input The ray, payload, and miss-record state described by `Context`.
    void invoke(in MissInput<Context> input);
}

/// Represents the source implementation of a callable stage.
///
/// `Context` fixes the callable-data and callable-record types. The compiler turns `invoke` into a
/// native callable entry point or the corresponding structural dispatch function.
///
/// @par Constraints
/// Requires the `structural_raytracing_callable` capability.
[require(structural_raytracing_callable)]
public interface ICallableShader
{
    /// Defines the callable-data and shader-record types available to this stage.
    ///
    /// @par Constraints
    /// Must conform to `ICallableContext`.
    associatedtype Context : ICallableContext;

    /// Implements the callable stage selected by `RayTracer.callShader`.
    /// @param[in] input The callable-data and callable-record state described by `Context`.
    void invoke(in CallableInput<Context> input);
}

/// Selects no closest-hit stage for a hit group.
///
/// Its empty `invoke` satisfies the source contract, while structural lowering omits the native
/// stage when the target does not need a placeholder function.
/// @tparam C The context shared by the hit group's stages.
///
/// @par Constraints
/// `C` must conform to `IHitContext`. This type conforms to `IClosestHitShader` with `Context = C`.
public struct NoClosestHit<C> : IClosestHitShader
    where C : IHitContext
{
    /// Uses the hit group's context for the absent closest-hit stage.
    ///
    /// @par Constraints
    /// Equals `C`, which conforms to `IHitContext`.
    public typealias Context = C;

    /// Satisfies the closest-hit shader contract without changing the hit or payload.
    /// @param[in] input The committed-hit state for `C`; this placeholder does not read it.
    public void invoke(in ClosestHitInput<C> input) {}
}

/// Selects no any-hit stage for a hit group.
///
/// Its empty `invoke` satisfies the source contract; structural lowering recognizes this exact
/// standard-module type and does not emit user any-hit behavior.
/// @tparam C The context shared by the hit group's stages.
///
/// @par Constraints
/// `C` must conform to `IHitContext`. This type conforms to `IAnyHitShader` with `Context = C`.
public struct NoAnyHit<C> : IAnyHitShader
    where C : IHitContext
{
    /// Uses the hit group's context for the absent any-hit stage.
    ///
    /// @par Constraints
    /// Equals `C`, which conforms to `IHitContext`.
    public typealias Context = C;

    /// Satisfies the any-hit shader contract without rejecting or terminating the candidate.
    /// @param[in] input The candidate-hit state for `C`; this placeholder does not read it.
    public void invoke(in AnyHitInput<C> input) {}
}

/// Selects hardware intersection instead of a user intersection stage.
///
/// Structural lowering recognizes this exact standard-module type and omits a procedural
/// intersection entry point. Unlike a real `IIntersectionShader`, the placeholder has no `invoke`.
/// @tparam C The context shared by the hit group's stages.
///
/// @par Constraints
/// `C` must conform to `IHitContext`, and `C.Primitive` must conform to
/// `IBuiltinIntersectionPrimitive`, selecting hardware triangle or curve intersection. This type
/// conforms to `IIntersectionStage` with `Context = C`.
public struct NoIntersection<C> : IIntersectionStage
    where C : IHitContext
    where C.Primitive : IBuiltinIntersectionPrimitive
{
    /// Uses the hit group's context for the hardware intersection stage.
    ///
    /// @par Constraints
    /// Equals `C`, which conforms to `IHitContext` and selects an `IBuiltinIntersectionPrimitive`.
    public typealias Context = C;
}

} // namespace rt
