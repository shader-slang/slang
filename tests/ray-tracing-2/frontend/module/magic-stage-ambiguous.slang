//DIAGNOSTIC_TEST:SIMPLE(diag=AMBIGUOUS): -experimental-feature -entry DualStage -no-codegen
//TEST:SIMPLE: -experimental-feature -entry DualStage -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry DualStage -stage callable -no-codegen
//TEST:SIMPLE: -experimental-feature -entry SharedInvoke -no-codegen

import slang.raytracing;
import magic_stage_library;
import magic_stage_secondary_library;

struct Traversal : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct SharedContext : rt::IPayloadContext, rt::ICallableContext
{
    typealias TraceContext = Traversal;
    typealias Payload = uint;
    typealias Record = void;
    typealias CallableData = uint;
}

// This struct implements two different standard stages. The command line must select one;
// the imported refinement of rt::IMissShader must preserve the same rule as direct conformance.
struct DualStage : first::IBackgroundStage, rt::ICallableShader
/*AMBIGUOUS:
       ^^^^^^^^^ ambiguous structural ray-tracing entry point stage
       ^^^^^^^^^ struct 'DualStage' implements more than one executable ray-tracing stage; use '-stage <name>' to select one
*/
{
    typealias Context = SharedContext;
    static void invoke(in rt::MissInput<Context> input) {}
    static void invoke(in rt::CallableInput<Context> input) {}
}

// Two application interfaces inherit the same rt::IMissShader requirement. Their witnesses
// select the same implementation, so they do not create a second stage or an ambiguity.
struct SharedInvoke : first::IBackgroundStage, second::IBackgroundStage
{
    typealias Context = SharedContext;
    static void invoke(in rt::MissInput<Context> input) {}
}
