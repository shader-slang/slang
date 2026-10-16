//TEST:SIMPLE: -experimental-feature -no-codegen

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

// A stage may have ordinary fields even when its stage interface is inherited from an
// imported application interface.
struct StatefulMiss : first::IBackgroundStage
{
    typealias Context = MissContext;
    uint state;

    static void invoke(in rt::MissInput<Context> input) {}
}
