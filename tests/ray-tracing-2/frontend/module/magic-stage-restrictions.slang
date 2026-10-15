//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing;
import magic_stage_library;

struct Traversal : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = Traversal;
    typealias Record = void;
    typealias Payload = uint;
}

// Refining rt::IMissShader in an imported application interface retains the standard stage
// restrictions, even when no entry point is explicitly selected.
struct StatefulMiss : first::IBackgroundStage
{
    typealias Context = MissContext;
    uint state;
//CHECK: ^^^^^ stateful structural ray-tracing stage
//CHECK: ^^^^^ instance field 'state' is not allowed because structural ray-tracing stage implementations must be stateless

    static void invoke(in rt::MissInput<Context> input) {}
}
