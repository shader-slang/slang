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

void useInput(Input input)
{
    // Explicit and inferred generic arguments can pass and copy an existing input.
    // Constructing a fresh input through a specialized generic is diagnosed separately, in
    // tests/ray-tracing-2/frontend/diagnostics/stage-input-generic-construction.slang.
    consume(input);
    consume<Input>(input);
    let inferredCopy = copy(input);
    let explicitCopy = copy<Input>(input);
    consume(inferredCopy);
    consume(explicitCopy);
}
