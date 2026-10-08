module structural_rt_mixed_linked_stage;

import slang.raytracing;
import structural_rt_legacy_helper;

public struct LinkedTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct LinkedMissContext : rt::IPayloadContext
{
    typealias TraceContext = LinkedTraceContext;
    typealias Payload = LegacyPayload;
    typealias Record = void;
}

RaytracingAccelerationStructure legacyScene;

// The schema explicitly lists this serialized stage. Its legacy helper call must remain visible
// to mixed-API validation after linking the modules.
public struct LinkedLegacyCallingMiss : rt::IMissShader
{
    typealias Context = LinkedMissContext;

    // Keep the exported function's stage contract even on targets with an empty body below.
    [require(structural_raytracing_miss)]
    void invoke(rt::MissInput<Context> input)
    {
        // The public stage also supports Metal, where the legacy TraceRay API is unavailable.
        // Keep the deliberately invalid mixed-API call on the native target exercised here.
        __target_switch
        {
        case hlsl:
            RayDesc ray = {};
            legacyTrace(legacyScene, ray, input.payload);
            break;
        default:
            break;
        }
    }
}
