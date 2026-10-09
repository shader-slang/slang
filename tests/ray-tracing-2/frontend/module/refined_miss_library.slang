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

public struct ExtensionMiss {}

public extension ExtensionMiss : IRefinedMiss
{
    public typealias Context = RefinedMissContext;
    public void invoke(in rt::MissInput<Context> input) {}
}

public struct GenericMissContext<P> : rt::IPayloadContext
{
    public typealias TraceContext = RefinedTraceContext;
    public typealias Record = void;
    public typealias Payload = P;
}

public struct GenericMiss<P> : IRefinedMiss
{
    public typealias Context = GenericMissContext<P>;
    public void invoke(in rt::MissInput<Context> input) {}
}

public interface IDefaultMiss : IRefinedMiss {}

public extension<T : IDefaultMiss> T
{
    public void invoke(in rt::MissInput<Context> input) {}
}

public struct DefaultMiss : IDefaultMiss
{
    public typealias Context = RefinedMissContext;
}

public interface IInvokeProvider
{
    associatedtype Context : rt::IPayloadContext;
}

public extension<T : IInvokeProvider> T
{
    public void invoke(in rt::MissInput<Context> input) {}
}

public struct SharedDefaultMiss : IInvokeProvider, IRefinedMiss
{
    public typealias Context = RefinedMissContext;
}

public struct OrdinaryInvokeProvider : IInvokeProvider
{
    public typealias Context = RefinedMissContext;
}
