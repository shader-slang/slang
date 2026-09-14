module structural_rt_mixed_open_stage;

import slang.raytracing;
import structural_rt_legacy_helper;

public struct OpenTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct OpenMissContext : rt::IPayloadContext
{
    typealias TraceContext = OpenTraceContext;
    typealias Payload = LegacyPayload;
    typealias Record = void;
}

public interface IOpenMissTag : rt::IMissShader
{
}

RaytracingAccelerationStructure legacyScene;

// This stage is absent from the schema source. Its tagged conformance adds it when the serialized
// plugin is linked, so open-section completion is the producer of the runtime-dispatch edge.
struct OpenLegacyCallingMiss : IOpenMissTag
{
    typealias Context = OpenMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        RayDesc ray = {};
        legacyTrace(legacyScene, ray, input.payload);
    }
}
