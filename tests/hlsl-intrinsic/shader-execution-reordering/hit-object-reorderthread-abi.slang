// Regression for #11903 / #11997: the coherence-hint-only ReorderThread(uint, uint) overload has no
// HitObject operand, so its HLSL ABI is decided entirely by its `__target_switch` / [require] gating.
// Native DXR MaybeReorderThread is an SM 6.9 intrinsic, so the native `hlsl` arm requires ser_dxr;
// NVAPI SER is the explicit nvapi_hit_objects opt-in. Confirm each config selects the matching
// intrinsic and never the other ABI's.

// [1] Native DXR (SM 6.9): dx::MaybeReorderThread, never the NVAPI intrinsic.
//TEST:SIMPLE(filecheck=NATIVE): -target hlsl -profile sm_6_9 -entry main -stage raygeneration -capability ser_dxr

// [2] Explicit NVAPI opt-in on a pre-6.9 profile: NvReorderThread, never the native intrinsic.
//TEST:SIMPLE(filecheck=NVAPI): -target hlsl -profile sm_6_5 -entry main -stage raygeneration -capability nvapi_hit_objects

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[shader("raygeneration")]
void main()
{
    ReorderThread(0u, 0u);
    // NATIVE: dx::MaybeReorderThread
    // NATIVE-NOT: NvReorderThread
    // NVAPI: NvReorderThread
    // NVAPI-NOT: dx::MaybeReorderThread
    outputBuffer[0] = 1u;
}
