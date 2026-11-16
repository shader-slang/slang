//TEST:SIMPLE: -experimental-feature -ignore-capabilities -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -ignore-capabilities -target metal -no-codegen

import slang.raytracing.portable;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Context : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

typealias Input = rt::MissInput<Context>;

T copy<T>(T value) { return value; }
void consume<T>(T value) {}

void make<T>()
{
    T fresh = T();
}

T makeValue<T>() { return T(); }

struct Factory<T>
{
    static void make()
    {
        T fresh = T();
    }
}

void useInput(Input input)
{
    // Explicit and inferred generic arguments can pass and copy an existing input.
    consume(input);
    consume<Input>(input);
    let inferredCopy = copy(input);
    let explicitCopy = copy<Input>(input);
    consume(inferredCopy);
    consume(explicitCopy);

    // These tests stop at frontend checking. The generic bodies only know T; validating
    // construction after T becomes a stage input is separate work after specialization.
    make<Input>();
    let fresh = makeValue<Input>();
    Factory<Input>::make();
}
