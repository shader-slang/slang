//TEST_IGNORE_FILE:

module magic_stage_library;

// The import supplies compiler marker constants, while this module declares its own contracts.
// Their annotations establish separate identities; their names do not grant compiler behavior.
import slang.raytracing;

namespace first
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
