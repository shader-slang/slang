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
    copiedAgain.payload.value += copiedAgain.record.value;
}
}
