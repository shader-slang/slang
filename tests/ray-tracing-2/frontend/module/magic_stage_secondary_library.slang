//TEST_IGNORE_FILE:

module magic_stage_secondary_library;

// A second module independently declares the same compiler-recognized roles. Loading it must
// not replace the first module's declarations or associated-type requirements in a global map.
namespace second
{
public interface ITraversal
{
    associatedtype AccelerationStructure;
    associatedtype Motion;
}

public interface IRecordContext
{
    associatedtype TraceContext : ITraversal;
    [KnownBuiltin(__RayTracingBuiltin.StageRecord)]
    associatedtype Record;
}

public interface IPayloadContext : IRecordContext
{
    [KnownBuiltin(__RayTracingBuiltin.PayloadContextPayload)]
    associatedtype Payload;
}

[require(structural_raytracing_miss)]
[KnownBuiltin(__RayTracingBuiltin.MissInput)]
public struct BackgroundInput<C : IPayloadContext> {}

[require(structural_raytracing_miss)]
__magic_type(MissShaderType)
public interface IBackgroundStage
{
    [KnownBuiltin(__RayTracingBuiltin.StageContext)]
    associatedtype Context : IPayloadContext;
    void invoke(in BackgroundInput<Context> input);
}
}
