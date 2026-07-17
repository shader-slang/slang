// Using HitObject on an HLSL target that has neither SM 6.9 (native DXR) nor the explicit
// nvapiHitObjects capability must produce a clean diagnostic, not an internal-error crash. #11903
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -profile sm_6_5 -entry main -stage raygeneration -capability hlsl_nvapi

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[shader("raygeneration")]
void main()
{
    // CHECK: 55215
    // CHECK: nvapiHitObjects
    HitObject h = HitObject::MakeNop();
    outputBuffer[0] = h.IsNop() ? 1u : 0u;
}
