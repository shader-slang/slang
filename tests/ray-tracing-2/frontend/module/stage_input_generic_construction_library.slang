// Library half of stage-input-generic-construction-module.slang: declares the generic helpers
// (in namespace generic_construction) whose bodies only become stage-input uses once a caller
// binds T. That test compiles this file to a .slang-module and imports it from source and
// serialized runs; the expectations live there, not here.
import slang.raytracing.portable;

namespace generic_construction
{
public struct Payload { uint value; }
public struct Record { uint value; }

public struct TraceContext : rt::ITraceContext
{
    public typealias AccelerationStructure = rt::AccelerationStructure;
    public typealias Motion = rt::NoMotion;
}

public struct Context : rt::IPayloadContext
{
    public typealias TraceContext = generic_construction::TraceContext;
    public typealias Payload = generic_construction::Payload;
    public typealias Record = generic_construction::Record;
}

// The library declares the generic helper; its body is only checked abstractly here, and the
// construction is diagnosed when a caller supplies a stage input as T.
public uint makeFreshInput<T>()
{
    T fresh = T();
    return 1;
}

// A chain of generic hops through the library must reach the same check.
public uint hopOne<T>()
{
    return makeFreshInput<T>();
}

public interface ICarrier { associatedtype Item; }

public struct InputCarrier : ICarrier
{
    typealias Item = rt::MissInput<Context>;
}

// A generic signature written in terms of T.Item stores an input once substituted, and the
// signature is part of the serialized module even when bodies are not.
public void acceptOptionalItem<T : ICarrier>(Optional<T.Item> input) {}
}
