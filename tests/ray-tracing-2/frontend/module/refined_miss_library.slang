//TEST_IGNORE_FILE:

module refined_miss_library;

import slang.raytracing;

public struct RefinedTraceContext : rt::ITraceContext
{
    public typealias AccelerationStructure = rt::AccelerationStructure;
    public typealias Motion = rt::NoMotion;
}

public struct RefinedMissContext : rt::IPayloadContext
{
    public typealias TraceContext = RefinedTraceContext;
    public typealias Record = void;
    public typealias Payload = uint;
}

public interface IRefinedMiss : rt::IMissShader {}

public struct RefinedMiss : IRefinedMiss
{
    public typealias Context = RefinedMissContext;
    public void invoke(in rt::MissInput<Context> input) {}
}
