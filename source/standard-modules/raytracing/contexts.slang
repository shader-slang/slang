implementing raytracing;

namespace rt
{

public interface ITraceContext
{
    associatedtype Payload;
    associatedtype AccelerationStructure : IAccelerationStructure;
    associatedtype Motion : IRayMotion;
}

public interface IHitContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype Primitive : IIntersectionPrimitive;
    associatedtype Record;
}

public interface IMissGroupContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype Record;
}

public interface ICallableGroupContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype CallableData;
    associatedtype Record;
}

} // namespace rt
