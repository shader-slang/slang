//TEST_IGNORE_FILE:

module magic_stage_secondary_library;

// A second module independently declares the same compiler-recognized roles. Loading it must
// not replace the first module's declarations or associated-type requirements in a global map.
// The import supplies marker constants without using the standard module's stage contracts.
import slang.raytracing;

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
    [KnownBuiltin(rt::__RayTracingBuiltin.StageRecord)]
    associatedtype Record;
}

public interface IPayloadContext : IRecordContext
{
    [KnownBuiltin(rt::__RayTracingBuiltin.PayloadContextPayload)]
    associatedtype Payload;
}

[require(structural_raytracing_miss)]
[KnownBuiltin(rt::__RayTracingBuiltin.MissInput)]
public struct BackgroundInput<C : IPayloadContext> {}

[require(structural_raytracing_miss)]
__magic_type(MissShaderType)
public interface IBackgroundStage
{
    [KnownBuiltin(rt::__RayTracingBuiltin.StageContext)]
    associatedtype Context : IPayloadContext;
    void invoke(in BackgroundInput<Context> input);
}
}
