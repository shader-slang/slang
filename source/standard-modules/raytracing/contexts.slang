implementing raytracing;

namespace rt
{

/// Describes traversal-wide state shared by every stage selected from one trace program schema.
///
/// Payload and shader-record types are stage-specific and therefore belong to stage contexts, not
/// this common traversal context.
public interface ITraceContext
{
    /// Selects the acceleration-structure topology used by the trace call and its stages.
    ///
    /// An interface lets this API support both the portable two-level BVH and targets such as
    /// Metal that also support multi-level BVHs through `MultiLevelAccelerationStructure`.
    ///
    /// @par Constraints
    /// Must conform to the sealed `IAccelerationStructure` interface. Select the portable
    /// `rt::AccelerationStructure` handle or a supported `MultiLevelAccelerationStructure<N>`.
    /// The selected type's target requirements also apply to this context.
    associatedtype AccelerationStructure : IAccelerationStructure;

    /// Selects whether traversal uses a ray time and which geometry can move.
    ///
    /// @par Constraints
    /// Must conform to the sealed `IRayMotion` interface. Select `NoMotion`, `PrimitiveMotion`,
    /// `InstanceMotion`, or `PrimitiveAndInstanceMotion`; the selected type's target and version
    /// requirements apply to this context. Stage-input `time` additionally requires conformance
    /// to `IEnabledRayMotion`.
    associatedtype Motion : IRayMotion;
}

/// Describes the context shared by closest-hit, any-hit, intersection, miss, and callable stages.
///
/// It defines two associated-type requirements:
/// - `TraceContext`: the type that selects the acceleration-structure topology and motion model
///   for the trace program. It must conform to `ITraceContext`.
/// - `Record`: the type of application-defined data associated with this stage's shader binding
///   table (SBT) entry and read through `input.record`. Use `void` when no record data is needed.
public interface IStageContext
{
    /// Names the trace call's context type, carried into each stage's type-level contract.
    ///
    /// This keeps the acceleration-structure topology and motion model consistent with the schema
    /// used by `RayTracer`; the context describes types rather than a runtime object.
    ///
    /// @par Constraints
    /// Must conform to `ITraceContext`. When a stage belongs to a program schema, its trace
    /// context must equal the schema's `TraceContext`; conformance to this interface alone does
    /// not enforce that cross-stage equality.
    associatedtype TraceContext : ITraceContext;

    /// Describes the host-supplied record bytes that this stage reads through `input.record`.
    ///
    /// @par Constraints
    /// This associated type has no conformance constraint. Use `void` when the stage has no
    /// record data. For a non-void type, the application must supply matching record data using
    /// the target's record layout; this declaration does not validate the host allocation.
    [KnownBuiltin(__RayTracingBuiltin.StageRecord)]
    associatedtype Record;
}

/// Describes a stage context whose stages read and write a ray payload.
///
/// Closest-hit, any-hit, and miss stages use this payload contract. Intersection stages can also
/// access the payload on Metal and CUDA/OptiX, where the target supports it.
/// A miss shader needs no primitive-specific state because traversal found no accepted hit, so
/// `IPayloadContext` is its complete contract and no separate `IMissContext` is needed. It may also
/// reuse an `IHitContext` when it shares that context's payload and record types.
public interface IPayloadContext : IStageContext
{
    /// Describes the mutable ray payload exchanged between a trace call and its hit or miss stage.
    ///
    /// @par Constraints
    /// This associated type has no conformance constraint. The trace call's payload and the
    /// receiving stage's payload must have the same type. Intersection-stage access additionally
    /// requires `structural_raytracing_intersection_payload` (Metal or CUDA/OptiX). Empty structs
    /// are valid payload types; the target ABI determines their physical storage.
    [KnownBuiltin(__RayTracingBuiltin.PayloadContextPayload)]
    associatedtype Payload;
}

/// Describes the primitive and payload contract shared by every stage in one hit group.
///
/// Conformance to `IPayloadContext` supplies the inherited `TraceContext`, `Record`, and `Payload`
/// requirements. The hit group requires its stage contexts to equal this complete context type.
public interface IHitContext : IPayloadContext
{
    /// Selects the geometry kind and the attributes exposed by the hit-stage inputs.
    ///
    /// @par Constraints
    /// Must conform to the sealed `IIntersectionPrimitive` interface. Select `TrianglePrimitive`,
    /// `CurvePrimitive`, or `BoundingBoxPrimitive<CustomAttributes>`. An intersection shader
    /// additionally requires `ICustomIntersectionPrimitive`; `NoIntersection` instead requires
    /// `IBuiltinIntersectionPrimitive`. Those stage constraints restrict this selection when a
    /// hit group is assembled.
    [KnownBuiltin(__RayTracingBuiltin.HitPrimitive)]
    associatedtype Primitive : IIntersectionPrimitive;
}

/// Describes the data and record types used by a callable stage.
///
/// Callable data is independent from ray payload data, but the context still names the common
/// traversal context so the schema can reject entries from a different trace program.
public interface ICallableContext : IStageContext
{
    /// Describes the mutable data supplied by shader code to `RayTracer.callShader`.
    /// The callable stage reads and updates that argument through `input.data`, and the caller
    /// observes those updates when the call returns.
    ///
    /// @par Constraints
    /// This associated type has no conformance constraint. The argument passed to `callShader`
    /// and the receiving callable stage must use the same callable-data type.
    [KnownBuiltin(__RayTracingBuiltin.CallableData)]
    associatedtype CallableData;
}

} // namespace rt
