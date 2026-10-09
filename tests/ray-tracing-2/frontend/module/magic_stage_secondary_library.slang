//TEST_IGNORE_FILE:

module magic_stage_secondary_library;

// A second module independently declares the same compiler-recognized roles. Loading it must
// not replace the first module's declarations or associated-type requirements in a global map.
namespace second
{
__magic_type(RayTracingTraceContextType)
public interface ITraversal
{
    associatedtype AccelerationStructure;
    associatedtype Motion;
}

__magic_type(RayTracingStageContextType)
public interface IRecordContext
{
    associatedtype TraceContext : ITraversal;
    associatedtype Record;
}

__magic_type(RayTracingPayloadContextType)
public interface IPayloadContext : IRecordContext
{
    associatedtype Payload;
}

[require(structural_raytracing_miss)]
__magic_type(MissInputType)
public struct BackgroundInput<C : IPayloadContext> {}

[require(structural_raytracing_miss)]
__magic_type(MissShaderType)
public interface IBackgroundStage
{
    associatedtype Context : IPayloadContext;
    void invoke(in BackgroundInput<Context> input);
}
}
