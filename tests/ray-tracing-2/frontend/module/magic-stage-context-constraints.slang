//TEST:SIMPLE: -experimental-feature -entry ExtraBound -no-codegen
//TEST:SIMPLE: -experimental-feature -entry RefinedPayloadBound -no-codegen

import slang.raytracing.portable;
import magic_stage_library;

interface IExtra {}
interface IRefinedPayloadContext : rt::IPayloadContext {}

// An application interface can constrain the Context inherited from the standard stage
// interface. Its additional bound does not replace the inherited payload and record requirements.
interface IExtraStage : first::IBackgroundStage
{
    __constraint Context : IExtra;
}

// A refined context bound carries the standard payload contract through ordinary inheritance.
interface IRefinedStage : first::IBackgroundStage
{
    __constraint Context : IRefinedPayloadContext;
}

struct Traversal : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct SharedContext : IExtra, IRefinedPayloadContext
{
    typealias TraceContext = Traversal;
    typealias Record = void;
    typealias Payload = uint;
}

struct ExtraBound : IExtraStage
{
    typealias Context = SharedContext;
    static void invoke(in rt::MissInput<Context> input) {}
}

struct RefinedPayloadBound : IRefinedStage
{
    typealias Context = SharedContext;
    static void invoke(in rt::MissInput<Context> input) {}
}
