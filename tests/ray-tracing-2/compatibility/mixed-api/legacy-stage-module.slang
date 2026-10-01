module structural_rt_legacy_stage;

public struct LegacyPayload
{
    uint value;
}

[shader("miss")]
public void LegacyMiss(inout LegacyPayload payload)
{
    payload.value = 1;
}
