//TEST:SIMPLE: -experimental-feature -entry CallingMiss -entry SharedMiss -target hlsl -no-codegen

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    static void invoke(in rt::ClosestHitInput<TestHitContext> input) {}

    // A static helper can pass an existing input to invoke. Neither spelling below creates
    // a stage object or a new input.
    static void forward(in rt::ClosestHitInput<TestHitContext> input)
    {
        TestClosestHit::invoke(input);
        invoke(input);
    }
}

// The standard module's no-op stage implementation also provides a static invoke method.
extension rt::NoClosestHit<TestHitContext>
{
    static void forward(in rt::ClosestHitInput<TestHitContext> input)
    {
        rt::NoClosestHit<TestHitContext>::invoke(input);
    }
}

struct TestMissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = uint;
    typealias Record = uint;
}

struct SharedMiss : rt::IMissShader
{
    typealias Context = TestMissContext;

    // Calling invoke needs no stage object. The input still refers to the caller's payload
    // and shader record.
    static void invoke(in rt::MissInput<Context> input)
    {
        input.payload = input.record;
    }
}

struct CallingMiss : rt::IMissShader
{
    typealias Context = TestMissContext;
    static void invoke(in rt::MissInput<Context> input)
    {
        SharedMiss::invoke(input);
    }
}

// This call resolves to the interface requirement before a concrete implementation is known.
extension<T : rt::IMissShader> T
{
    static void forward(in rt::MissInput<Context> input)
    {
        T::invoke(input);
    }
}
