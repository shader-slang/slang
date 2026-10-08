// Companion to hit-object-invoke-nvapi-error.slang (#11903): the DXR-1.3-native-only 2-argument
// HitObject::Invoke(hitObject, payload) must compile cleanly on plain SM 6.9 (no ser_nvapi),
// emitting the native dx::HitObject::Invoke. This is the positive counterpart to the NVAPI-target
// error case — it confirms the fix only diagnoses the NVAPI target and leaves the native path
// unchanged.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_9

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct MyPayload { float4 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    MyPayload payload = { float4(0) };

    // The HitObject type lowers to the native dx::HitObject (not NvHitObject).
    // CHECK: dx::HitObject hitFromNop
    HitObject hitFromNop = HitObject::MakeNop();

    // The 2-argument Invoke lowers to the native dx::HitObject::Invoke on SM 6.9.
    // CHECK: dx::HitObject::Invoke
    HitObject::Invoke(hitFromNop, payload);

    outputBuffer[idx] = hitFromNop.IsNop() ? 1 : 0;
}
