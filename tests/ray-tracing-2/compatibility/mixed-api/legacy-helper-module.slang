module structural_rt_legacy_helper;

public struct LegacyPayload
{
    public uint value;
}

public void legacyTrace(
    RaytracingAccelerationStructure scene,
    RayDesc ray,
    inout LegacyPayload payload)
{
    TraceRay(scene, 0, 0xff, 0, 0, 0, ray, payload);
}
