import slang.raytracing;

namespace input_copy
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
    public typealias TraceContext = input_copy::TraceContext;
    public typealias Payload = input_copy::Payload;
    public typealias Record = input_copy::Record;
}

[require(structural_raytracing_miss)]
public void updatePayload(in rt::MissInput<Context> input)
{
    rt::MissInput<Context> local = input;
    let copiedAgain = local;
    // Saving and reimporting this module must preserve wrapping and unwrapping the same view.
    Optional<rt::MissInput<Context>> maybe = copiedAgain;
    if (maybe.hasValue)
    {
        let unwrapped = maybe.value;
        unwrapped.payload.value += unwrapped.record.value;
    }
}
}
