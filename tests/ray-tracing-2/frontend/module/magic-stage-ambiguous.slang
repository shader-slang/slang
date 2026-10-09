//DIAGNOSTIC_TEST:SIMPLE(filecheck=AMBIGUOUS): -experimental-feature -entry DualMiss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=AMBIGUOUS): -experimental-feature -entry DualMiss -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=AMBIGUOUS): -experimental-feature -entry DualStage -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry DualStage -stage callable -no-codegen
//TEST:SIMPLE: -experimental-feature -entry SharedInvoke -no-codegen

import slang.raytracing;
import magic_stage_library;
import magic_stage_secondary_library;

interface ICallableContext : first::IRecordContext
{
    [KnownBuiltin(rt::__RayTracingBuiltin.CallableData)]
    associatedtype CallableData;
}

[require(structural_raytracing_callable)]
[KnownBuiltin(rt::__RayTracingBuiltin.CallableInput)]
struct CallableInput<C : ICallableContext> {}

[require(structural_raytracing_callable)]
__magic_type(CallableShaderType)
interface ICallableStage
{
    [KnownBuiltin(rt::__RayTracingBuiltin.StageContext)]
    associatedtype Context : ICallableContext;
    void invoke(in CallableInput<Context> input);
}

struct Traversal : first::ITraversal, second::ITraversal
{
    typealias AccelerationStructure = uint;
    typealias Motion = void;
}

struct SharedContext : first::IPayloadContext, second::IPayloadContext, ICallableContext
{
    typealias TraceContext = Traversal;
    typealias Payload = uint;
    typealias Record = void;
    typealias CallableData = uint;
}

// Both interfaces denote a miss stage, but selecting that stage cannot choose between its bodies.
struct DualMiss : first::IBackgroundStage, second::IBackgroundStage
{
    typealias Context = SharedContext;
    void invoke(in first::BackgroundInput<Context> input) {}
    void invoke(in second::BackgroundInput<Context> input) {}
}

// An ambiguous miss stage must not prevent explicitly selecting a separate, unique callable stage.
struct DualStage : first::IBackgroundStage, second::IBackgroundStage, ICallableStage
{
    typealias Context = SharedContext;
    void invoke(in first::BackgroundInput<Context> input) {}
    void invoke(in second::BackgroundInput<Context> input) {}
    void invoke(in CallableInput<Context> input) {}
}

[require(structural_raytracing_miss)]
__magic_type(MissShaderType)
interface ISharedMiss
{
    [KnownBuiltin(rt::__RayTracingBuiltin.StageContext)]
    associatedtype Context : first::IPayloadContext;
    void invoke(in first::BackgroundInput<Context> input);
}

// Repeated stage contracts are unambiguous when their checked witnesses select the same method.
struct SharedInvoke : first::IBackgroundStage, ISharedMiss
{
    typealias Context = SharedContext;
    void invoke(in first::BackgroundInput<Context> input) {}
}

// AMBIGUOUS: multiple 'invoke' implementations for the 'miss' stage
