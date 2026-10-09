//TEST:SIMPLE: -experimental-feature -entry ExtraBoundFirst -no-codegen
//TEST:SIMPLE: -experimental-feature -entry PayloadBoundFirst -no-codegen
//TEST:SIMPLE: -experimental-feature -entry RefinedPayloadBound -no-codegen

import magic_stage_library;

interface IExtra {}
interface IRefinedPayloadContext : first::IPayloadContext {}

// Additional bounds must not change which witness supplies the stage's payload and record.
[require(structural_raytracing_miss)]
__magic_type(MissShaderType)
interface IExtraFirstStage
{
    [KnownBuiltin(__RayTracingBuiltin.StageContext)]
    associatedtype Context : IExtra;
    __constraint Context : first::IPayloadContext;
    void invoke(in first::BackgroundInput<Context> input);
}

[require(structural_raytracing_miss)]
__magic_type(MissShaderType)
interface IPayloadFirstStage
{
    [KnownBuiltin(__RayTracingBuiltin.StageContext)]
    associatedtype Context : first::IPayloadContext;
    __constraint Context : IExtra;
    void invoke(in first::BackgroundInput<Context> input);
}

// A refined bound carries the payload contract through ordinary interface inheritance.
[require(structural_raytracing_miss)]
__magic_type(MissShaderType)
interface IRefinedStage
{
    [KnownBuiltin(__RayTracingBuiltin.StageContext)]
    associatedtype Context : IExtra;
    __constraint Context : IRefinedPayloadContext;
    void invoke(in first::BackgroundInput<Context> input);
}

struct Traversal : first::ITraversal
{
    typealias AccelerationStructure = uint;
    typealias Motion = void;
}

struct SharedContext : IExtra, IRefinedPayloadContext
{
    typealias TraceContext = Traversal;
    typealias Record = void;
    typealias Payload = uint;
}

struct ExtraBoundFirst : IExtraFirstStage
{
    typealias Context = SharedContext;
    void invoke(in first::BackgroundInput<Context> input) {}
}

struct PayloadBoundFirst : IPayloadFirstStage
{
    typealias Context = SharedContext;
    void invoke(in first::BackgroundInput<Context> input) {}
}

struct RefinedPayloadBound : IRefinedStage
{
    typealias Context = SharedContext;
    void invoke(in first::BackgroundInput<Context> input) {}
}
