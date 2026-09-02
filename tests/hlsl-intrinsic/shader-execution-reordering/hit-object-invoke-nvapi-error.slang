// Test that the DXR-1.3-native-only 2-argument HitObject::Invoke(hitObject, payload) is
// diagnosed cleanly when targeting NVAPI shader-execution-reordering, instead of silently
// emitting `dx::HitObject::Invoke` applied to an `NvHitObject` (invalid HLSL). See #11903.
//
// The NVAPI Invoke requires an AccelerationStructure (3-argument form), so the 2-argument
// form has no NVAPI equivalent. On `-capability nvapiHitObjects` the HitObject type emits as
// NvHitObject, so selecting the native `dx::HitObject::Invoke` overload for it is invalid;
// the fix adds `case nvapiHitObjects: static_assert(false, ...)` to the 2-argument overload's
// __target_switch, which the target-switch specializer selects for an NVAPI target.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_9 -capability nvapiHitObjects

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct MyPayload { float4 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    MyPayload payload = { float4(0) };

    HitObject hitFromNop = HitObject::MakeNop();

    // The 2-argument Invoke has no NVAPI form; expect a clean error rather than invalid HLSL.
    //CHECK: 41400
    //CHECK: has no NVAPI form
    HitObject::Invoke(hitFromNop, payload);

    outputBuffer[idx] = hitFromNop.IsNop() ? 1 : 0;
}
