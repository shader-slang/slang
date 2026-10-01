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
    associatedtype AccelerationStructure : IAccelerationStructure;
    associatedtype Motion : IRayMotion;
}

/// Describes the context shared by closest-hit, any-hit, intersection, miss, and callable stages.
///
/// `Record` is the application-defined data selected with the stage's SBT entry. A target may store
/// those bytes in the native record or address backend-owned storage through a local binding.
/// Different stage contexts may share the same `TraceContext` while using different records.
public interface IStageContext
{
    /// Names the trace call's context type, carried into each stage's type-level contract.
    ///
    /// This keeps the acceleration-structure topology and motion model consistent with the schema
    /// used by `RayTracer`; the context describes types rather than a runtime object.
    associatedtype TraceContext : ITraceContext;

    /// Describes the host-supplied record bytes that this stage reads through `input.record`.
    /// Use `void` when the stage has no record data.
    associatedtype Record;
}

/// Describes a stage context whose stages read and write a ray payload.
///
/// Closest-hit, any-hit, and miss stages use this payload contract. An intersection stage shares
/// the hit group's `IHitContext`, but its stage input does not expose the payload.
/// A miss shader needs no primitive-specific state because traversal found no accepted hit, so
/// `IPayloadContext` is its complete contract and no separate `IMissContext` is needed. It may also
/// reuse an `IHitContext` when it shares that context's payload and record types.
public interface IPayloadContext : IStageContext
{
    associatedtype Payload;
}

/// Describes the primitive and payload contract shared by every stage in one hit group.
public interface IHitContext : IPayloadContext
{
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
    associatedtype CallableData;
}

} // namespace rt
