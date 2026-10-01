module structural_rt_generic_interface_legacy_helper;

public struct LegacyPayload
{
    public uint value;
}

public interface ILegacyTrace
{
    void trace(inout LegacyPayload payload);
}

RaytracingAccelerationStructure legacyScene;

public struct LegacyTrace : ILegacyTrace
{
    public void trace(inout LegacyPayload payload)
    {
        RayDesc ray = {};
        TraceRay(legacyScene, 0, 0xff, 0, 0, 0, ray, payload);
    }
}

public void
invokeLegacyTrace<T>(T implementation, inout LegacyPayload payload) where T : ILegacyTrace
{
    // Before generic specialization this call is represented by an interface-witness lookup, not
    // a direct reference to `LegacyTrace.trace`.
    implementation.trace(payload);
}
