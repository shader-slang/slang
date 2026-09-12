implementing raytracing;

namespace rt
{

/// Describes traversal-wide state shared by every stage selected from one trace program schema.
///
/// Payload and shader-record types are stage-specific and therefore belong to stage contexts, not
/// this common traversal context.
public interface ITraceContext
{
    associatedtype AccelerationStructure;
    __constraint AccelerationStructure : IAccelerationStructure;
    associatedtype Motion;
    __constraint Motion : IRayMotion;
}

/// Describes state common to every executable structural ray-tracing stage.
///
/// `Record` is the application-defined data appended to the native SBT record that selected the
/// stage. Different stage contexts may share the same `TraceContext` while using different records.
public interface IStageContext
{
    associatedtype TraceContext;
    __constraint TraceContext : ITraceContext;
    associatedtype Record;
}

/// Describes a stage context whose stages read and write a ray payload.
///
/// Both hit and miss stages use this contract. A miss shader may reuse an `IHitContext` when it
/// intentionally shares that context's payload and record types.
public interface IPayloadContext : IStageContext
{
    associatedtype Payload;
}

/// Describes the primitive and payload contract shared by every stage in one hit group.
public interface IHitContext : IPayloadContext
{
    associatedtype Primitive;
    __constraint Primitive : IIntersectionPrimitive;
}

/// Describes the data and record types used by a callable stage.
///
/// Callable data is independent from ray payload data, but the context still names the common
/// traversal context so the schema can reject entries from a different trace program.
public interface ICallableContext : IStageContext
{
    associatedtype CallableData;
}

} // namespace rt
