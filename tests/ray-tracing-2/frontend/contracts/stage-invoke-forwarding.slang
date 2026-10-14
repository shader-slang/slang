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
    void invoke(in rt::ClosestHitInput<TestHitContext> input) {}

    // An existing receiver can call its stage method with an existing input, just as it can
    // call any other helper. Neither spelling below constructs a stage object or a new input.
    void forward(in rt::ClosestHitInput<TestHitContext> input)
    {
        this.invoke(input);
        invoke(input);
    }
}

// The standard module's no-op stage implementation can also be called as an ordinary method.
extension rt::NoClosestHit<TestHitContext>
{
    void forward(in rt::ClosestHitInput<TestHitContext> input)
    {
        this.invoke(input);
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

    // A static method can satisfy the stage interface's method requirement. Calling it needs
    // no stage receiver, and the input still refers to the caller's payload and shader record.
    static void invoke(in rt::MissInput<Context> input)
    {
        input.payload = input.record;
    }
}

struct CallingMiss : rt::IMissShader
{
    typealias Context = TestMissContext;
    void invoke(in rt::MissInput<Context> input)
    {
        SharedMiss::invoke(input);
    }
}

// This call resolves to the interface requirement before a concrete implementation is known.
extension<T : rt::IMissShader> T
{
    void forward(in rt::MissInput<Context> input)
    {
        this.invoke(input);
    }
}
