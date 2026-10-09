//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import magic_stage_library;

struct Traversal : first::ITraversal
{
    typealias AccelerationStructure = uint;
    typealias Motion = void;
}

struct MissContext : first::IPayloadContext
{
    typealias TraceContext = Traversal;
    typealias Record = void;
    typealias Payload = uint;
}

// The magic type also supplies the stage's semantic restrictions without a standard-module
// import or an explicitly selected entry point.
struct StatefulMiss : first::IBackgroundStage
{
    typealias Context = MissContext;
    uint state;
//CHECK: ^^^^^ stateful structural ray-tracing stage
//CHECK: ^^^^^ instance field 'state' is not allowed because structural ray-tracing stage implementations are compiler-created

    void invoke(in first::BackgroundInput<Context> input) {}
}

struct RecursiveMiss : first::IBackgroundStage
{
    typealias Context = MissContext;
    void invoke(in first::BackgroundInput<Context> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}
